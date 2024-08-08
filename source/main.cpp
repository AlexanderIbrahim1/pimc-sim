#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <tuple>

#include <tomlplusplus/toml.hpp>
#include <torch/script.h>

#include <argparser.hpp>
#include <common/writers/writer_utils.hpp>
#include <constants/constants.hpp>
#include <coordinates/coordinates.hpp>
#include <environment/environment.hpp>
#include <estimators/pimc/centroid.hpp>
#include <estimators/pimc/centroid_radial_distribution_function.hpp>
#include <estimators/pimc/primitive_kinetic.hpp>
#include <estimators/pimc/radial_distribution_function.hpp>
#include <estimators/pimc/three_body_potential.hpp>
#include <estimators/pimc/two_body_potential.hpp>
#include <estimators/writers/default_writers.hpp>
#include <geometries/bravais.hpp>
#include <geometries/lattice.hpp>
#include <geometries/lattice_type.hpp>
#include <geometries/unit_cell_translations.hpp>
#include <interactions/four_body/rescaling.hpp>
#include <interactions/handlers/composite_interaction_handler.hpp>
#include <interactions/handlers/full_interaction_handler.hpp>
#include <interactions/handlers/interaction_handler_concepts.hpp>
#include <interactions/handlers/nearest_neighbour_interaction_handler.hpp>
#include <interactions/three_body/three_body_parah2.hpp>
#include <interactions/three_body/three_body_pointwise_wrapper.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
#include <interactions/two_body/two_body_pointwise_tabulated.hpp>
#include <interactions/two_body/two_body_pointwise_wrapper.hpp>
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

constexpr auto NDIM = std::size_t {3};

constexpr auto build_hcp_lattice_structure(auto density)
{
    /* create the lattice positions and the periodic box */
    const auto lattice_type = geom::LatticeType::HCP;
    const auto lattice_constant = geom::density_to_lattice_constant(density, lattice_type);
    const auto hcp_unit_cell = geom::conventional_hcp_unit_cell(lattice_constant);
    const auto hcp_unit_cell_box = geom::unit_cell_box_sides(hcp_unit_cell);
    const auto lattice_box_translations = geom::UnitCellTranslations<NDIM> {5ul, 3ul, 3ul};
    const auto minimage_box = geom::lattice_box(hcp_unit_cell_box, lattice_box_translations);

    const auto lattice_site_positions = geom::lattice_particle_positions(hcp_unit_cell, lattice_box_translations);
    const auto n_particles = lattice_site_positions.size();

    return std::tuple(n_particles, minimage_box, lattice_site_positions);
}

constexpr auto lennard_jones_parah2_potential(auto minimage_box)
{
    /*
        Parameters for the Lennard-Jones potential are taken from paragraph 3 of page 354
        of `Eur. Phys. J. D 56, 353–358 (2010)`. Original units are in Kelvin and Angstroms,
        converted to wavenumbers and angstroms.
    */

    const auto distance_pot = interact::LennardJonesPotential {23.77, 2.96};
    const auto pot = interact::PeriodicTwoBodyPointPotential {distance_pot, minimage_box};

    return pot;
}

auto fsh_potential(auto minimage_box)
{
    const auto fsh_dirpath = std::filesystem::path {"/home/a68ibrah/research/simulations/pimc-sim/potentials"};
    const auto fsh_filename = "fsh_potential_angstroms_wavenumbers.potext_sq";
    auto distance_pot = interact::create_fsh_pair_potential<double>(fsh_dirpath / fsh_filename);
    const auto pot = interact::PeriodicTwoBodySquaredPointPotential {std::move(distance_pot), minimage_box};

    return pot;
}

auto load_trilinear_interpolator(const std::filesystem::path& data_filepath)
{
    auto instream = std::ifstream(data_filepath, std::ios::in);
    if (!instream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Error: Unable to open file for trilinear interpolation data: '" << data_filepath << "'\n";
        throw std::ios_base::failure {err_msg.str()};
    }

    common_utils::writer_utils::skip_lines_starting_with(instream, '#');

    // arguments (r, s, u) correspond to coordinates (R, s, cos(phi)) in the published paper
    std::size_t r_size, s_size, u_size;
    instream >> r_size;
    instream >> s_size;
    instream >> u_size;

    double r_min, r_max, s_min, s_max, u_min, u_max;
    instream >> r_min;
    instream >> r_max;
    instream >> s_min;
    instream >> s_max;
    instream >> u_min;
    instream >> u_max;

    const auto shape = mathtools::Shape3D {r_size, s_size, u_size};
    const auto r_limits = mathtools_utils::AxisLimits {r_min, r_max};
    const auto s_limits = mathtools_utils::AxisLimits {s_min, s_max};
    const auto u_limits = mathtools_utils::AxisLimits {u_min, u_max};

    // read in all the energies into a vector
    const auto n_elements = r_size * s_size * u_size;
    auto energies = std::vector<double> {};
    energies.reserve(n_elements);

    double energy;
    for (std::size_t i {0}; i < n_elements; ++i) {
        instream >> energy;
        energies.push_back(energy);
    }

    // create the 3D grid of energies to perform trilinear interpolation on
    auto grid = mathtools::Grid3D {std::move(energies), shape};

    return mathtools::TrilinearInterpolator<double> {std::move(grid), r_limits, s_limits, u_limits};
}

auto threebodyparah2_potential(auto minimage_box)
{
    const auto pot_dirpath = std::filesystem::path {"/home/a68ibrah/research/simulations/pimc-sim/playground/scripts"};
    const auto pot_filename = "threebody_126_101_51.dat";
    const auto c9_coefficient = 34336.2;  // [cm]^[-1] * [Angstrom]^[9]

    auto interpolator = load_trilinear_interpolator(pot_dirpath / pot_filename);
    auto distance_pot = interact::ThreeBodyParaH2Potential {std::move(interpolator), c9_coefficient};

    return interact::PeriodicThreeBodyPointPotential {std::move(distance_pot), minimage_box};
}

auto create_com_move_adjuster(double lower_range_limit, double upper_range_limit) noexcept
    -> pimc::SingleValueMoveAdjuster<double>
{
    const auto com_accept_range = pimc::AcceptPercentageRange<double> {lower_range_limit, upper_range_limit};
    const auto com_adjust_step = double {0.01};
    const auto com_direction = pimc::DirectionIfAcceptTooLow::NEGATIVE;
    const auto com_limits = pimc::MoveLimits<double> {0.0, std::nullopt};
    return pimc::SingleValueMoveAdjuster<double> {com_accept_range, com_adjust_step, com_direction, com_limits};
}

auto create_bisect_move_adjuster(double lower_range_limit, double upper_range_limit) noexcept
    -> pimc::BisectionLevelMoveAdjuster<double>
{
    const auto com_accept_range = pimc::AcceptPercentageRange<double> {lower_range_limit, upper_range_limit};
    const auto com_adjust_step = double {0.1};
    return pimc::BisectionLevelMoveAdjuster<double> {com_accept_range, com_adjust_step};
}

auto create_histogram(
    const std::filesystem::path& histogram_filepath,
    const sim::ContinueFileManager& manager,
    const coord::BoxSides<double, NDIM>& minimage_box
)
{
    if (manager.is_continued()) {
        return mathtools::io::read_histogram<double>(histogram_filepath);
    }
    else {
        return mathtools::Histogram<double> {0.0, coord::box_cutoff_distance(minimage_box), 1024};
    }
}

auto main() -> int
{
    const auto rescaling_limits = interact::rescale::RescalingLimits {1.0, 2.0, 3.0, 4.0};
    const auto rescaling_function = interact::rescale::RescalingFunction {1.0, 1.0, 1.0};
    const auto forward_rescaler = interact::rescale::ForwardEnergyRescaler {rescaling_function, rescaling_limits};

    const auto side_length_groups = torch::tensor({{1.0, 2.0, 3.0, 4.0, 5.0, 6.0}}, torch::dtype(torch::kFloat64));
    auto energies_to_rescale = torch::tensor({{6.0}}, torch::dtype(torch::kFloat64));

    interact::rescale::forward_rescale_energies(forward_rescaler, side_length_groups, energies_to_rescale);

    std::cout << "RESULT: " << energies_to_rescale[0].item<double>() << '\n';
    std::exit(EXIT_SUCCESS);

    namespace fs = std::filesystem;

    const auto output_dirpath = fs::path {"/home/a68ibrah/research/simulations/pimc-sim/playground/ignore"};

    const auto toml_input = std::string_view {R"(
        first_block_index = 0
        last_block_index = 200
        n_equilibrium_blocks = 10
        n_passes = 2
        n_timeslices = 32
        centre_of_mass_step_size = 0.3
        bisection_level = 3
        bisection_ratio = 0.5
        density = 0.026
        temperature = 4.2
    )"};

    const auto n_most_recent_worldlines_to_save = 5;

    const auto continue_file_manager = sim::ContinueFileManager {output_dirpath};
    auto toml_stream = std::stringstream {std::string {toml_input}};

    const auto parser = argparse::ArgParser {toml_stream};
    if (!parser.is_valid()) {
        std::cout << "PARSER DID NOT PARSE PROPERLY\n";
        std::exit(EXIT_FAILURE);
    }

    const auto temperature = parser.temperature;
    const auto n_timeslices = parser.n_timeslices;
    const auto com_step_size = parser.centre_of_mass_step_size;
    const auto bisect_move_info = pimc::BisectionLevelMoveInfo {parser.bisection_ratio, parser.bisection_level};

    const auto last_block_index = parser.last_block_index;
    const auto first_block_index = [&]()
    {
        if (continue_file_manager.file_exists()) {
            return continue_file_manager.parse_block_index();
        }
        else {
            return parser.first_block_index;
        }
    }();

    const auto [n_particles, minimage_box, lattice_site_positions] = build_hcp_lattice_structure(parser.density);

    /* create the worldlines and worldline writer*/
    auto worldline_writer = worldline::WorldlineWriter<double, NDIM> {output_dirpath};

    auto worldlines = [&]()
    {
        if (continue_file_manager.is_continued()) {
            const auto worldline_filepath = worldline_writer.output_filepath(first_block_index);
            return worldline::read_worldlines<double, NDIM>(worldline_filepath);
        }
        else {
            return worldline::worldlines_from_positions<double, NDIM>(lattice_site_positions, n_timeslices);
        }
    }();

    sim::write_box_sides(output_dirpath / "box_sides.dat", minimage_box);

    const auto pot = fsh_potential(minimage_box);
    const auto pot3b = threebodyparah2_potential(minimage_box);

    /* create the environment object */
    const auto h2_mass = constants::H2_MASS_IN_AMU<double>;
    const auto environment = envir::create_environment(temperature, h2_mass, n_timeslices, n_particles);

    /* create the interaction handler */

    // using PairInteractionHandler = interact::FullPairInteractionHandler<decltype(pot), double, NDIM>;
    // using TripletInteractionHandler = interact::FullTripletInteractionHandler<decltype(pot3b), double, NDIM>;
    // using InteractionHandler = interact::CompositeFullInteractionHandler<double, NDIM, PairInteractionHandler,
    // TripletInteractionHandler>;

    using PairInteractionHandler = interact::NearestNeighbourPairInteractionHandler<decltype(pot), double, NDIM>;
    using TripletInteractionHandler =
        interact::NearestNeighbourTripletInteractionHandler<decltype(pot3b), double, NDIM>;
    using InteractionHandler = interact::
        CompositeNearestNeighbourInteractionHandler<double, NDIM, PairInteractionHandler, TripletInteractionHandler>;

    const auto lattice_constant = geom::density_to_lattice_constant(parser.density, geom::LatticeType::HCP);
    const auto pair_cutoff_distance = 2.2 * lattice_constant;
    const auto triplet_cutoff_distance = 1.1 * lattice_constant;

    // using InteractionHandler = interact::NearestNeighbourPairInteractionHandler<decltype(pot), double, NDIM>;
    // auto interaction_handler = InteractionHandler {pot, n_particles};
    // interact::update_centroid_adjacency_matrix<double, NDIM>(
    //     worldlines, minimage_box, environment, interaction_handler.adjacency_matrix(), pair_cutoff_distance
    // );

    auto pair_interaction_handler = PairInteractionHandler {pot, n_particles};
    auto triplet_interaction_handler = TripletInteractionHandler {std::move(pot3b), n_particles};
    auto interaction_handler =
        InteractionHandler {std::move(pair_interaction_handler), std::move(triplet_interaction_handler)};

    interact::update_centroid_adjacency_matrix<double, NDIM>(
        worldlines, minimage_box, environment, interaction_handler.adjacency_matrix<0>(), pair_cutoff_distance
    );

    interact::update_centroid_adjacency_matrix<double, NDIM>(
        worldlines, minimage_box, environment, interaction_handler.adjacency_matrix<1>(), triplet_cutoff_distance
    );

    /* create the PRNG; save the seed (or set it?) */
    auto prngw = rng::RandomNumberGeneratorWrapper<std::mt19937>::from_random_uint64();

    /* create the move performers */
    auto com_mover = pimc::CentreOfMassMovePerformer<double, NDIM> {n_timeslices, com_step_size};
    auto single_bead_mover = pimc::SingleBeadPositionMovePerformer<double, NDIM> {n_timeslices};
    auto multi_bead_mover = pimc::BisectionMultibeadPositionMovePerformer<double, NDIM> {bisect_move_info};

    /* create the move adjusters */
    const auto com_move_adjuster = create_com_move_adjuster(0.4, 0.5);
    const auto bisect_move_adjuster = create_bisect_move_adjuster(0.4, 0.5);

    auto com_step_size_writer = pimc::default_centre_of_mass_position_move_step_size_writer<double>(output_dirpath);
    auto multi_bead_move_info_writer =
        pimc::default_bisection_multibead_position_move_info_writer<double>(output_dirpath);

    /* create the move acceptance rate trackers for the move performers */
    auto com_tracker = pimc::MoveSuccessTracker {};
    auto single_bead_tracker = pimc::MoveSuccessTracker {};
    auto multi_bead_tracker = pimc::MoveSuccessTracker {};

    auto com_move_writer = pimc::default_centre_of_mass_position_move_success_writer(output_dirpath);
    auto single_bead_move_writer = pimc::default_single_bead_position_move_success_writer(output_dirpath);
    auto multi_bead_move_writer = pimc::default_bisection_multibead_position_move_success_writer(output_dirpath);

    /* create the file writers for the estimators */
    auto kinetic_writer = estim::default_kinetic_writer<double>(output_dirpath);
    auto pair_potential_writer = estim::default_pair_potential_writer<double>(output_dirpath);
    auto triplet_potential_writer = estim::default_triplet_potential_writer<double>(output_dirpath);
    auto rms_centroid_writer = estim::default_rms_centroid_distance_writer<double>(output_dirpath);
    auto abs_centroid_writer = estim::default_absolute_centroid_distance_writer<double>(output_dirpath);

    /* create the histogram and the histogram writers */
    const auto radial_dist_histo_filepath = output_dirpath / "radial_dist_histo.dat";
    auto radial_dist_histo = create_histogram(radial_dist_histo_filepath, continue_file_manager, minimage_box);

    const auto centroid_dist_histo_filepath = output_dirpath / "centroid_radial_dist_histo.dat";
    auto centroid_dist_histo = create_histogram(centroid_dist_histo_filepath, continue_file_manager, minimage_box);

    const auto periodic_distance_calculator = coord::PeriodicDistanceMeasureWrapper<double, NDIM> {minimage_box};

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

        // interact::update_centroid_adjacency_matrix<double, 3>(
        //     worldlines, minimage_box, environment, interaction_handler.adjacency_matrix(), cutoff_distance
        // );

        if (i_block >= parser.n_equilibrium_blocks) {
            const auto& threebody_pot = interaction_handler.get<1>();

            /* run estimators */
            const auto total_kinetic_energy = estim::total_primitive_kinetic_energy(worldlines, environment);
            const auto total_pair_potential_energy =
                estim::total_pair_potential_energy_periodic(worldlines, pot, environment);
            const auto total_triplet_potential_energy = estim::total_triplet_potential_energy_periodic(
                worldlines, threebody_pot.point_potential(), environment
            );
            const auto rms_centroid_dist = estim::rms_centroid_distance(worldlines, environment);
            const auto abs_centroid_dist = estim::absolute_centroid_distance(worldlines, environment);

            /* save estimators */
            kinetic_writer.write(i_block, total_kinetic_energy);
            pair_potential_writer.write(i_block, total_pair_potential_energy);
            triplet_potential_writer.write(i_block, total_triplet_potential_energy);
            rms_centroid_writer.write(i_block, rms_centroid_dist);
            abs_centroid_writer.write(i_block, abs_centroid_dist);

            /* save radial distribution function histogram */
            estim::update_radial_distribution_function_histogram(
                radial_dist_histo, periodic_distance_calculator, worldlines
            );
            mathtools::io::write_histogram(radial_dist_histo_filepath, radial_dist_histo);

            estim::update_centroid_radial_distribution_function_histogram(
                centroid_dist_histo, environment, periodic_distance_calculator, worldlines
            );
            mathtools::io::write_histogram(centroid_dist_histo_filepath, centroid_dist_histo);

            /* save move acceptance rates */
            const auto [com_accept, com_reject] = com_tracker.get_accept_and_reject();
            com_move_writer.write(i_block, com_accept, com_reject);

            const auto [sb_accept, sb_reject] = single_bead_tracker.get_accept_and_reject();
            single_bead_move_writer.write(i_block, sb_accept, sb_reject);

            const auto [mb_accept, mb_reject] = multi_bead_tracker.get_accept_and_reject();
            multi_bead_move_writer.write(i_block, mb_accept, mb_reject);

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
            const auto new_bisect_move_info =
                bisect_move_adjuster.adjust_step(curr_bisect_move_info, multi_bead_tracker);
            multi_bead_mover.update_bisection_level_move_info(new_bisect_move_info);

            multi_bead_move_info_writer.write(
                i_block, new_bisect_move_info.upper_level_frac, new_bisect_move_info.lower_level
            );
        }

        com_tracker.reset();
        single_bead_tracker.reset();
        multi_bead_tracker.reset();

        worldline::delete_worldlines_file<double, NDIM>(worldline_writer, i_block, n_most_recent_worldlines_to_save);
    }

    return 0;
}
