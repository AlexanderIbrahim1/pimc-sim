#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <string_view>

#include <tomlplusplus/toml.hpp>
#include <torch/script.h>

#include <argparser.hpp>
#include <constants/constants.hpp>
#include <coordinates/coordinates.hpp>
#include <environment/environment.hpp>
#include <estimators/pimc/centroid.hpp>
#include <estimators/pimc/centroid_radial_distribution_function.hpp>
#include <estimators/pimc/four_body_potential.hpp>
#include <estimators/pimc/primitive_kinetic.hpp>
#include <estimators/pimc/radial_distribution_function.hpp>
#include <estimators/pimc/three_body_potential.hpp>
#include <estimators/pimc/two_body_potential.hpp>
#include <estimators/writers/default_writers.hpp>
#include <geometries/bravais.hpp>
#include <geometries/lattice.hpp>
#include <geometries/lattice_type.hpp>
#include <geometries/unit_cell_translations.hpp>
#include <interactions/four_body/published_potential.hpp>
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
#include <rng/prng_state.hpp>
#include <simulation/box_sides_writer.hpp>
#include <simulation/continue.hpp>
#include <simulation/timer.hpp>
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

    const auto toml_input_filename = argv[1];
    const auto parser = argparse::ArgParser<float> {toml_input_filename};
    if (!parser.is_valid()) {
        std::cout << "ERROR: argument parser did not parse properly\n";
        std::cout << parser.error_message() << '\n';
        std::exit(EXIT_FAILURE);
    }

    const auto output_dirpath = parser.abs_output_dirpath;

    auto continue_file_manager = sim::ContinueFileManager {output_dirpath};
    if (continue_file_manager.file_exists()) {
        continue_file_manager.deserialize();
    }

    const auto temperature = parser.temperature;
    const auto n_timeslices = parser.n_timeslices;
    const auto com_step_size = parser.centre_of_mass_step_size;
    const auto bisect_move_info = pimc::BisectionLevelMoveInfo {parser.bisection_ratio, parser.bisection_level};

    // clang-format off
    const auto last_block_index = parser.last_block_index;
    const auto first_block_index = read_simulation_first_block_index(continue_file_manager, parser);

    const auto [n_particles, minimage_box, lattice_site_positions] = build_hcp_lattice_structure(parser.density, parser.n_unit_cells);

    const auto periodic_distance_calculator = coord::PeriodicDistanceMeasureWrapper<float, NDIM> {minimage_box};
    const auto periodic_distance_squared_calculator = coord::PeriodicDistanceSquaredMeasureWrapper<float, NDIM> {minimage_box};
    // clang-format on

    /* create the worldlines and worldline writer*/
    auto worldline_writer = worldline::WorldlineWriter<float, NDIM> {output_dirpath};
    auto worldlines = read_simulation_worldlines(continue_file_manager, worldline_writer, n_timeslices, lattice_site_positions);

    sim::write_box_sides(output_dirpath / "box_sides.dat", minimage_box);

    const auto pot = fsh_potential<float>(minimage_box, parser.abs_two_body_filepath);
    const auto pot3b = threebodyparah2_potential(minimage_box, parser.abs_three_body_filepath);

    const long int buffer_size = 1024;
    auto pot4b = interact::get_published_buffered_four_body_potential<NDIM, interact::PermutationTransformerFlag::EXACT>(parser.abs_four_body_filepath, buffer_size);
    // clang-format on

    /* create the environment object */
    const auto h2_mass = constants::H2_MASS_IN_AMU<float>;
    const auto environment = envir::create_environment(temperature, h2_mass, n_timeslices, n_particles);

    /* create the interaction handler */

    // clang-format off
    using PairInteractionHandler = interact::NearestNeighbourPairInteractionHandler<decltype(pot), float, NDIM>;
    using TripletInteractionHandler = interact::NearestNeighbourTripletInteractionHandler<decltype(pot3b), float, NDIM>;
    using QuadrupletInteractionHandler = interact::NearestNeighbourQuadrupletInteractionHandler<decltype(pot4b), float, NDIM>;
    using InteractionHandler = interact::CompositeNearestNeighbourInteractionHandler<float, NDIM, PairInteractionHandler, TripletInteractionHandler, QuadrupletInteractionHandler>;

    auto pair_interaction_handler = PairInteractionHandler {pot, n_particles};
    auto triplet_interaction_handler = TripletInteractionHandler {std::move(pot3b), n_particles};
    auto quadruplet_interaction_handler = QuadrupletInteractionHandler {std::move(pot4b), n_particles};
    auto interaction_handler = InteractionHandler {
        std::move(pair_interaction_handler),
        std::move(triplet_interaction_handler),
        std::move(quadruplet_interaction_handler)
    };
    // clang-format on

    const auto lattice_constant = geom::density_to_lattice_constant(parser.density, geom::LatticeType::HCP);
    const auto pair_cutoff_distance = static_cast<float>(2.2f * lattice_constant);
    const auto triplet_cutoff_distance = static_cast<float>(1.1f * lattice_constant);
    const auto quadruplet_cutoff_distance = static_cast<float>(1.1f * lattice_constant);

    interact::update_centroid_adjacency_matrix<float, NDIM>(
        worldlines, periodic_distance_squared_calculator, interaction_handler.adjacency_matrix<0>(), pair_cutoff_distance
    );

    interact::update_centroid_adjacency_matrix<float, NDIM>(
        worldlines, periodic_distance_squared_calculator, interaction_handler.adjacency_matrix<1>(), triplet_cutoff_distance
    );

    interact::update_centroid_adjacency_matrix<float, NDIM>(
        worldlines, periodic_distance_squared_calculator, interaction_handler.adjacency_matrix<2>(), quadruplet_cutoff_distance
    );

    /* create the PRNG; save the seed (or set it?) */
    const auto prng_state_filepath = rng::default_prng_state_filepath(output_dirpath);
    auto prngw = create_prngw(prng_state_filepath, parser.initial_seed_state);

    /* create the move performers */
    auto com_mover = pimc::CentreOfMassMovePerformer<float, NDIM> {n_timeslices, com_step_size};
    auto single_bead_mover = pimc::SingleBeadPositionMovePerformer<float, NDIM> {n_timeslices};
    auto multi_bead_mover = pimc::BisectionMultibeadPositionMovePerformer<float, NDIM> {bisect_move_info};

    /* create the move adjusters */
    const auto com_move_adjuster = create_com_move_adjuster<float>(0.3f, 0.4f);
    const auto bisect_move_adjuster = create_bisect_move_adjuster<float>(0.3f, 0.4f, 0.01f);

    // clang-format off
    auto com_step_size_writer = pimc::default_centre_of_mass_position_move_step_size_writer<float>(output_dirpath);
    auto multi_bead_move_info_writer = pimc::default_bisection_multibead_position_move_info_writer<float>(output_dirpath);
    // clang-format on

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
    auto triplet_potential_writer = estim::default_triplet_potential_writer<float>(output_dirpath);
    // auto quadruplet_potential_writer = estim::default_quadruplet_potential_writer<float>(output_dirpath);
    auto rms_centroid_writer = estim::default_rms_centroid_distance_writer<float>(output_dirpath);
    auto abs_centroid_writer = estim::default_absolute_centroid_distance_writer<float>(output_dirpath);

    /* create the histogram and the histogram writers */
    const auto radial_dist_histo_filepath = output_dirpath / "radial_dist_histo.dat";
    auto radial_dist_histo = create_histogram(radial_dist_histo_filepath, continue_file_manager, minimage_box);

    const auto centroid_dist_histo_filepath = output_dirpath / "centroid_radial_dist_histo.dat";
    auto centroid_dist_histo = create_histogram(centroid_dist_histo_filepath, continue_file_manager, minimage_box);

    /* create the timer and the corresponding writer to keep track of how long each block takes */
    auto timer = sim::Timer {};
    auto timer_writer = sim::default_timer_writer(output_dirpath);

    auto i_most_recent_saved_worldline = std::optional<std::size_t> {std::nullopt};

    const auto write_estimates = [&]() {
        kinetic_writer.write_and_clear();
        pair_potential_writer.write_and_clear();
        triplet_potential_writer.write_and_clear();
        // quadruplet_potential_writer.write_and_clear();
        rms_centroid_writer.write_and_clear();
        abs_centroid_writer.write_and_clear();
    };

    const auto write_moves = [&]() {
        com_move_writer.write_and_clear();
        single_bead_move_writer.write_and_clear();
        multi_bead_move_writer.write_and_clear();
        com_step_size_writer.write_and_clear();
        multi_bead_move_info_writer.write_and_clear();
    };

    const auto write_timer = [&]() {
        timer_writer.write_and_clear();
    };

    const auto write_histograms = [&]() {
        mathtools::io::write_histogram(radial_dist_histo_filepath, radial_dist_histo);
        mathtools::io::write_histogram(centroid_dist_histo_filepath, centroid_dist_histo);
    };

    const auto write_continue_and_prng = [&](std::size_t i_block) {
        /* create or update the continue file */
        if (i_most_recent_saved_worldline) {
            const auto worldline_index = i_most_recent_saved_worldline.value();
            continue_file_manager.set_info_and_serialize({i_block, worldline_index, true, i_block >= parser.n_equilibrium_blocks});
        } else {
            continue_file_manager.set_info_and_serialize({i_block, 0, false, i_block >= parser.n_equilibrium_blocks});
        }

        rng::save_prng_state(prngw.prng(), prng_state_filepath);
    };

    /* perform the simulation loop */
    for (std::size_t i_block {first_block_index}; i_block < last_block_index; ++i_block) {
        timer.start();
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

        /* save move acceptance rates */
        const auto [com_accept, com_reject] = com_tracker.get_accept_and_reject();
        com_move_writer.accumulate({i_block, com_accept, com_reject});

        const auto [sb_accept, sb_reject] = single_bead_tracker.get_accept_and_reject();
        single_bead_move_writer.accumulate({i_block, sb_accept, sb_reject});

        const auto [mb_accept, mb_reject] = multi_bead_tracker.get_accept_and_reject();
        multi_bead_move_writer.accumulate({i_block, mb_accept, mb_reject});

        // clang-format off
        if (i_block >= parser.n_equilibrium_blocks) {
            const auto& threebody_pot = interaction_handler.get<1>();

            // const auto box_cutoff = coord::box_cutoff_distance(minimage_box);
            // auto& fourbody_pot = interaction_handler.get<2>();

            /* run estimators */
            const auto total_kinetic_energy = estim::total_primitive_kinetic_energy(worldlines, environment);
            const auto total_pair_potential_energy = estim::total_pair_potential_energy_periodic(worldlines, pot);
            const auto total_triplet_potential_energy = estim::total_triplet_potential_energy_periodic(worldlines, threebody_pot.point_potential());
            // const auto total_quadruplet_potential_energy = estim::total_quadruplet_potential_energy_periodic(worldlines, fourbody_pot.point_potential(), minimage_box, box_cutoff);
            const auto rms_centroid_dist = estim::rms_centroid_distance(worldlines);
            const auto abs_centroid_dist = estim::absolute_centroid_distance(worldlines);

            /* accumulate estimators */
            kinetic_writer.accumulate({i_block, total_kinetic_energy});
            pair_potential_writer.accumulate({i_block, total_pair_potential_energy});
            triplet_potential_writer.accumulate({i_block, total_triplet_potential_energy});
            // quadruplet_potential_writer.accumulate({i_block, total_quadruplet_potential_energy});
            rms_centroid_writer.accumulate({i_block, rms_centroid_dist});
            abs_centroid_writer.accumulate({i_block, abs_centroid_dist});

            /* update radial distribution function histogram */
            estim::update_radial_distribution_function_histogram(radial_dist_histo, periodic_distance_calculator, worldlines);
            estim::update_centroid_radial_distribution_function_histogram(centroid_dist_histo, periodic_distance_calculator, worldlines);

            /* save the worldlines */
            if (parser.save_worldlines && ((i_block % parser.n_save_worldlines_every) == 0)) {
                worldline_writer.write(i_block, worldlines);
                i_most_recent_saved_worldline = i_block;
            }
        }
        /* Maybe update the step sizes during equilibration */
        if (i_block < parser.n_equilibrium_blocks && !parser.freeze_monte_carlo_step_sizes_in_equilibrium) {
            const auto curr_com_step_size = com_mover.step_size();
            const auto new_com_step_size = com_move_adjuster.adjust_step(curr_com_step_size, com_tracker);
            com_mover.update_step_size(new_com_step_size);

            const auto curr_bisect_move_info = multi_bead_mover.bisection_level_move_info();
            const auto new_bisect_move_info = bisect_move_adjuster.adjust_step(curr_bisect_move_info, multi_bead_tracker);
            multi_bead_mover.update_bisection_level_move_info(new_bisect_move_info);

            com_step_size_writer.accumulate({i_block, new_com_step_size});
            multi_bead_move_info_writer.accumulate({i_block, new_bisect_move_info.upper_level_frac, new_bisect_move_info.lower_level});
        }
        // clang-format on

        com_tracker.reset();
        single_bead_tracker.reset();
        multi_bead_tracker.reset();

        const auto duration = timer.duration_since_last_start();
        timer_writer.accumulate({i_block, duration.seconds, duration.milliseconds, duration.microseconds});

        /* write out the batch of estimates and update the histogram files */
        if ((i_block % parser.writer_batch_size) == 0) {
            write_estimates();
            write_moves();
            write_histograms();
            write_timer();
            write_continue_and_prng(i_block);
        }
    }

    write_estimates();
    write_moves();
    write_histograms();
    write_timer();
    write_continue_and_prng(last_block_index);

    return 0;
}
