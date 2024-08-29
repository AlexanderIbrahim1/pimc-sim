#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include <tomlplusplus/toml.hpp>
// #include <torch/script.h>

#include <argparser.hpp>
#include <common/writers/writer_utils.hpp>
#include <constants/constants.hpp>
#include <coordinates/coordinates.hpp>
#include <environment/environment.hpp>
#include <estimators/pimc/centroid.hpp>
#include <estimators/pimc/centroid_radial_distribution_function.hpp>
#include <estimators/pimc/primitive_kinetic.hpp>
#include <estimators/pimc/radial_distribution_function.hpp>
#include <estimators/pimc/two_body_potential.hpp>
#include <estimators/pimc/three_body_potential.hpp>
// #include <estimators/pimc/four_body_potential.hpp>
#include <estimators/writers/default_writers.hpp>
#include <geometries/bravais.hpp>
#include <geometries/lattice.hpp>
#include <geometries/lattice_type.hpp>
#include <geometries/unit_cell_translations.hpp>
// #include <interactions/four_body/published_potential.hpp>
#include <interactions/handlers/composite_interaction_handler.hpp>
#include <interactions/handlers/full_interaction_handler.hpp>
#include <interactions/handlers/interaction_handler_concepts.hpp>
#include <interactions/handlers/nearest_neighbour_interaction_handler.hpp>
#include <mathtools/grid/grid3d.hpp>
#include <mathtools/histogram/histogram.hpp>
#include <mathtools/interpolate/trilinear_interp.hpp>
#include <mathtools/io/histogram.hpp>
#include <pimc/adjusters/adjusters.hpp>
#include <pimc/bisection_multibead_position_move_performer.hpp>
#include <pimc/centre_of_mass_move.hpp>
#include <pimc/single_bead_position_move.hpp>
#include <pimc/trackers/move_success_tracker.hpp>
#include <pimc/writers/default_writers.hpp>
#include <rng/distributions.hpp>
#include <rng/generator.hpp>
#include <simulation/box_sides_writer.hpp>
#include <simulation/continue.hpp>
#include <worldline/worldline.hpp>
#include <worldline/writers/delete_worldlines.hpp>
#include <worldline/writers/read_worldlines.hpp>
#include <worldline/writers/worldline_writer.hpp>

#include <helper.cpp>

constexpr auto NDIM = std::size_t {3};

auto main(int argc, char** argv) -> int
{
    if (argc != 2) {
        std::cout << "ERROR: program incorrectly called from command line.\n";
        std::cout << "a.out path-to-toml-file\n";
        std::exit(EXIT_FAILURE);
    }

    const auto n_most_recent_worldlines_to_save = 5;

    const auto toml_input_filename = argv[1];
    const auto parser = argparse::ArgParser<float> {toml_input_filename};
    if (!parser.is_valid()) {
        std::cout << "PARSER DID NOT PARSE PROPERLY\n";
        std::cout << parser.error_message() << '\n';
        std::exit(EXIT_FAILURE);
    }

    const auto output_dirpath = parser.abs_output_dirpath;
    const auto continue_file_manager = sim::ContinueFileManager {output_dirpath};

    const auto temperature = parser.temperature;
    const auto n_timeslices = parser.n_timeslices;
    const auto com_step_size = parser.centre_of_mass_step_size;
    const auto bisect_move_info = pimc::BisectionLevelMoveInfo {parser.bisection_ratio, parser.bisection_level};

    const auto last_block_index = parser.last_block_index;
    const auto first_block_index = read_simulation_first_block_index(continue_file_manager, parser);

    const auto [n_particles, minimage_box, lattice_site_positions] = build_hcp_lattice_structure(parser.density);

    // clang-format off

    /* create the worldlines and worldline writer*/
    auto worldline_writer = worldline::WorldlineWriter<float, NDIM> {output_dirpath};
    auto worldlines = read_simulation_worldlines<NDIM>(continue_file_manager, worldline_writer, first_block_index, n_timeslices, lattice_site_positions);

    sim::write_box_sides(output_dirpath / "box_sides.dat", minimage_box);

    const auto pot = fsh_potential(minimage_box, parser.abs_two_body_filepath);
    // const auto pot3b = threebodyparah2_potential(minimage_box, parser.abs_three_body_filepath);

    // const auto abs_pot4b_filepath = std::filesystem::path {"/home/a68ibrah/research/simulations/pimc-sim/playground/scripts/models/fourbodypara_ssp_64_128_128_64_cpu_eval.pt"};
    // const long int buffer_size = 1024;
    // auto pot4b = interact::get_published_buffered_four_body_potential<NDIM, interact::PermutationTransformerFlag::EXACT>(parser.abs_four_body_filepath, buffer_size);
    // clang-format on

    /* create the environment object */
    const auto h2_mass = constants::H2_MASS_IN_AMU<float>;
    const auto environment = envir::create_environment(temperature, h2_mass, n_timeslices, n_particles);

    /* create the interaction handler */

    // clang-format off
    using InteractionHandler = interact::NearestNeighbourPairInteractionHandler<decltype(pot), float, NDIM>;
    // using PairInteractionHandler = interact::NearestNeighbourPairInteractionHandler<decltype(pot), float, NDIM>;
    // using TripletInteractionHandler = interact::NearestNeighbourTripletInteractionHandler<decltype(pot3b), float, NDIM>;
    // using QuadrupletInteractionHandler = interact::NearestNeighbourQuadrupletInteractionHandler<decltype(pot4b), float, NDIM>;
    // using InteractionHandler = interact::CompositeNearestNeighbourInteractionHandler<float, NDIM, PairInteractionHandler, TripletInteractionHandler>;
    // using InteractionHandler = interact::CompositeNearestNeighbourInteractionHandler<float, NDIM, PairInteractionHandler, TripletInteractionHandler, QuadrupletInteractionHandler>;

    auto interaction_handler = InteractionHandler {pot, n_particles};
    // auto pair_interaction_handler = PairInteractionHandler {pot, n_particles};
    // auto triplet_interaction_handler = TripletInteractionHandler {std::move(pot3b), n_particles};
    // auto quadruplet_interaction_handler = QuadrupletInteractionHandler {std::move(pot4b), n_particles};
    // auto interaction_handler = InteractionHandler {std::move(pair_interaction_handler), std::move(triplet_interaction_handler), std::move(quadruplet_interaction_handler)};
    // auto interaction_handler = InteractionHandler {std::move(pair_interaction_handler), std::move(triplet_interaction_handler)};
    // clang-format on

    const auto lattice_constant = geom::density_to_lattice_constant(parser.density, geom::LatticeType::HCP);
    const auto pair_cutoff_distance = static_cast<float>(2.2 * lattice_constant);
    // const auto triplet_cutoff_distance = static_cast<float>(1.1 * lattice_constant);
    // const auto quadruplet_cutoff_distance = static_cast<float>(1.1 * lattice_constant);

    interact::update_centroid_adjacency_matrix<float, NDIM>(
        worldlines, minimage_box, environment, interaction_handler.adjacency_matrix(), pair_cutoff_distance
    );

    // interact::update_centroid_adjacency_matrix<float, NDIM>(
    //     worldlines, minimage_box, environment, interaction_handler.adjacency_matrix<1>(), triplet_cutoff_distance
    // );

    // interact::update_centroid_adjacency_matrix<float, NDIM>(
    //     worldlines, minimage_box, environment, interaction_handler.adjacency_matrix<2>(), quadruplet_cutoff_distance
    // );

    /* create the PRNG; save the seed (or set it?) */
    auto prngw = rng::RandomNumberGeneratorWrapper<std::mt19937>::from_random_uint64();

    /* create the move performers */
    auto com_mover = pimc::CentreOfMassMovePerformer<float, NDIM> {n_timeslices, com_step_size};
    auto single_bead_mover = pimc::SingleBeadPositionMovePerformer<float, NDIM> {n_timeslices};
    auto multi_bead_mover = pimc::BisectionMultibeadPositionMovePerformer<float, NDIM> {bisect_move_info};

    /* create the move adjusters */
    const auto com_move_adjuster = create_com_move_adjuster(0.3f, 0.4f);
    const auto bisect_move_adjuster = create_bisect_move_adjuster(0.3f, 0.4f);

    auto com_step_size_writer = pimc::default_centre_of_mass_position_move_step_size_writer<float>(output_dirpath);
    auto multi_bead_move_info_writer = pimc::default_bisection_multibead_position_move_info_writer<float>(output_dirpath);

    /* create the move acceptance rate trackers for the move performers */
    auto com_tracker = pimc::MoveSuccessTracker {};
    auto single_bead_tracker = pimc::MoveSuccessTracker {};
    auto multi_bead_tracker = pimc::MoveSuccessTracker {};

    auto com_move_writer = pimc::default_centre_of_mass_position_move_success_writer(output_dirpath);
    auto single_bead_move_writer = pimc::default_single_bead_position_move_success_writer(output_dirpath);
    auto multi_bead_move_writer = pimc::default_bisection_multibead_position_move_success_writer(output_dirpath);

    /* create the file writers for the estimators */
    auto kinetic_writer = estim::default_kinetic_writer<float>(output_dirpath);
    auto pair_potential_writer = estim::default_pair_potential_writer<float>(output_dirpath);
    // auto triplet_potential_writer = estim::default_triplet_potential_writer<float>(output_dirpath);
    // auto quadruplet_potential_writer = estim::default_quadruplet_potential_writer<float>(output_dirpath);
    auto rms_centroid_writer = estim::default_rms_centroid_distance_writer<float>(output_dirpath);
    auto abs_centroid_writer = estim::default_absolute_centroid_distance_writer<float>(output_dirpath);

    /* create the histogram and the histogram writers */
    const auto radial_dist_histo_filepath = output_dirpath / "radial_dist_histo.dat";
    auto radial_dist_histo = create_histogram<NDIM>(radial_dist_histo_filepath, continue_file_manager, minimage_box);

    const auto centroid_dist_histo_filepath = output_dirpath / "centroid_radial_dist_histo.dat";
    auto centroid_dist_histo = create_histogram(centroid_dist_histo_filepath, continue_file_manager, minimage_box);

    const auto periodic_distance_calculator = coord::PeriodicDistanceMeasureWrapper<float, NDIM> {minimage_box};

    /* perform the simulation loop */
    for (std::size_t i_block {first_block_index}; i_block < last_block_index; ++i_block) {
        std::cout << "i_block = " << i_block << '\n';
        /* the number of passes is chosen such that the autocorrelation time between blocks is passed */
        for (std::size_t i_pass {0}; i_pass < parser.n_passes; ++i_pass) {
            /* perform COM move for each particle */
            for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
                com_mover(i_part, worldlines, prngw, interaction_handler, environment, &com_tracker);

                for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
                    /* perform bead move on timeslice `i_tslice` of each particle */
                    single_bead_mover(
                        i_part, i_tslice, worldlines, prngw, interaction_handler, environment, &single_bead_tracker
                    );
                }

                for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
                    /* perform bead move on timeslice `i_tslice` of each particle */
                    multi_bead_mover(
                        i_part, i_tslice, worldlines, prngw, interaction_handler, environment, &multi_bead_tracker
                    );
                }
            }
        }

        // interact::update_centroid_adjacency_matrix<float, 3>(
        //     worldlines, minimage_box, environment, interaction_handler.adjacency_matrix(), cutoff_distance
        // );

        /* save move acceptance rates */
        const auto [com_accept, com_reject] = com_tracker.get_accept_and_reject();
        com_move_writer.write(i_block, com_accept, com_reject);

        const auto [sb_accept, sb_reject] = single_bead_tracker.get_accept_and_reject();
        single_bead_move_writer.write(i_block, sb_accept, sb_reject);

        const auto [mb_accept, mb_reject] = multi_bead_tracker.get_accept_and_reject();
        multi_bead_move_writer.write(i_block, mb_accept, mb_reject);

        // clang-format off
        if (i_block >= parser.n_equilibrium_blocks) {
            // const auto& threebody_pot = interaction_handler.get<1>();
            // auto& fourbody_pot = interaction_handler.get<2>();
            // auto& fourbody_pot = pot4b;

            /* run estimators */
            const auto total_kinetic_energy = estim::total_primitive_kinetic_energy(worldlines, environment);
            const auto total_pair_potential_energy = estim::total_pair_potential_energy_periodic(worldlines, pot, environment);
            // const auto total_triplet_potential_energy = estim::total_triplet_potential_energy_periodic(worldlines, threebody_pot.point_potential(), environment);
            // const auto total_quadruplet_potential_energy = estim::calculate_total_four_body_potential_energy_via_shifting(worldlines, fourbody_pot, environment, minimage_box, coord::box_cutoff_distance(minimage_box));
            const auto rms_centroid_dist = estim::rms_centroid_distance(worldlines, environment);
            const auto abs_centroid_dist = estim::absolute_centroid_distance(worldlines, environment);

            /* save estimators */
            kinetic_writer.write(i_block, total_kinetic_energy);
            pair_potential_writer.write(i_block, total_pair_potential_energy);
            // triplet_potential_writer.write(i_block, total_triplet_potential_energy);
            // quadruplet_potential_writer.write(i_block, total_quadruplet_potential_energy);
            rms_centroid_writer.write(i_block, rms_centroid_dist);
            abs_centroid_writer.write(i_block, abs_centroid_dist);

            /* save radial distribution function histogram */
            estim::update_radial_distribution_function_histogram(radial_dist_histo, periodic_distance_calculator, worldlines);
            mathtools::io::write_histogram(radial_dist_histo_filepath, radial_dist_histo);

            estim::update_centroid_radial_distribution_function_histogram(centroid_dist_histo, environment, periodic_distance_calculator, worldlines);
            mathtools::io::write_histogram(centroid_dist_histo_filepath, centroid_dist_histo);

            /* save the worldlines */
            worldline_writer.write(i_block, worldlines, environment);

            /* create the continue file */
            continue_file_manager.serialize_block_index(i_block);
        }

        /* Update the step sizes during equilibration */
        if (i_block < parser.n_equilibrium_blocks) {
            const auto curr_com_step_size = com_mover.step_size();
            const auto new_com_step_size = com_move_adjuster.adjust_step(curr_com_step_size, com_tracker);
            com_mover.update_step_size(new_com_step_size);

            com_step_size_writer.write(i_block, new_com_step_size);

            const auto curr_bisect_move_info = multi_bead_mover.bisection_level_move_info();
            const auto new_bisect_move_info = bisect_move_adjuster.adjust_step(curr_bisect_move_info, multi_bead_tracker);
            multi_bead_mover.update_bisection_level_move_info(new_bisect_move_info);

            multi_bead_move_info_writer.write(i_block, new_bisect_move_info.upper_level_frac, new_bisect_move_info.lower_level);
        }
        // clang-format on

        com_tracker.reset();
        single_bead_tracker.reset();
        multi_bead_tracker.reset();

        worldline::delete_worldlines_file<float, NDIM>(worldline_writer, i_block, n_most_recent_worldlines_to_save);
    }

    return 0;
}
