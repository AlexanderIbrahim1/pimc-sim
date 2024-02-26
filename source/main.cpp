#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <tuple>

#include <tomlplusplus/toml.hpp>

#include <argparser.hpp>
#include <constants/constants.hpp>
#include <coordinates/box_sides.hpp>
#include <coordinates/coordinates.hpp>
#include <environment/environment.hpp>
#include <estimators/pimc/centroid.hpp>
#include <estimators/pimc/primitive_kinetic.hpp>
#include <estimators/pimc/two_body_potential.hpp>
#include <estimators/writers/default_writers.hpp>
#include <estimators/writers/single_value_writer.hpp>
#include <geometries/bravais.hpp>
#include <geometries/lattice.hpp>
#include <geometries/lattice_type.hpp>
#include <geometries/unit_cell_translations.hpp>
#include <interactions/handlers/full_pair_interaction_handler.hpp>
#include <interactions/handlers/interaction_handler_concepts.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
#include <interactions/two_body/two_body_pointwise_tabulated.hpp>
#include <interactions/two_body/two_body_pointwise_wrapper.hpp>
#include <pimc/centre_of_mass_move.hpp>
#include <pimc/single_bead_position_move.hpp>
#include <rng/distributions.hpp>
#include <rng/generator.hpp>
#include <worldline/worldline.hpp>

/*
PLAN for main

What might I read in later?
- information needed to create the potentials
- nearest neighbour info
*/

/*
TODO:
- create function to make worldlines from the lattice site positions
- create the functions used with the movers (step generators, etc.)
- create the single-bead mover
*/

constexpr auto NDIM = std::size_t {3};

constexpr auto build_hcp_lattice_structure(auto density)
{
    /* create the lattice positions and the periodic box */
    const auto lattice_type = geom::LatticeType::HCP;
    const auto lattice_constant = geom::density_to_lattice_constant(density, lattice_type);
    const auto hcp_unit_cell = geom::conventional_hcp_unit_cell(lattice_constant);
    const auto hcp_unit_cell_box = geom::unit_cell_box_sides(hcp_unit_cell);
    const auto lattice_box_translations = geom::UnitCellTranslations<NDIM> {2ul, 2ul, 2ul};
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
    const auto pot = interact::PeriodicPairDistancePotential {distance_pot, minimage_box};

    return pot;
}

constexpr auto fsh_potential(auto minimage_box)
{
    const auto fsh_dirpath = std::filesystem::path {"/home/a68ibrah/research/simulations/pimc-sim/potentials"};
    const auto fsh_filename = "fsh_potential_angstroms_wavenumbers.potext_sq";
    const auto distance_pot = interact::create_fsh_pair_potential<double>(fsh_dirpath / fsh_filename);
    const auto pot = interact::PeriodicPairDistanceSquaredPotential {distance_pot, minimage_box};

    return pot;
}

auto main() -> int
{
    namespace fs = std::filesystem;

    const auto toml_input = std::string_view {R"(
        first_block_index = 0
        last_block_index = 200
        n_equilibrium_blocks = 20
        n_passes = 10
        n_timeslices = 32
        centre_of_mass_step_size = 0.3
        bisection_level = 3
        bisection_ratio = 0.4
        density = 0.026
        temperature = 4.2
    )"};

    auto toml_stream = std::stringstream {std::string {toml_input}};
    const auto parser = argparse::ArgParser {toml_stream};

    const auto temperature = parser.temperature;
    const auto n_timeslices = parser.n_timeslices;
    const auto com_step_size = parser.centre_of_mass_step_size;

    if (!parser.is_valid()) {
        std::cout << "PARSER DID NOT PARSE PROPERLY\n";
        std::exit(EXIT_FAILURE);
    }

    const auto [n_particles, minimage_box, lattice_site_positions] = build_hcp_lattice_structure(parser.density);
    auto worldlines = worldline::worldlines_from_positions<double, NDIM>(lattice_site_positions, n_timeslices);

    const auto pot = fsh_potential(minimage_box);

    /* create the interaction handler */
    const auto interaction_handler = interact::FullPairInteractionHandler<decltype(pot), double, NDIM> {pot};

    /* create the environment object */
    const auto h2_mass = constants::H2_MASS_IN_AMU<double>;
    const auto environment =
        envir::create_finite_temperature_environment(temperature, h2_mass, n_timeslices, n_particles);

    /* create the PRNG; save the seed (or set it?) */
    auto prngw = rng::RandomNumberGeneratorWrapper<std::mt19937>::from_random_uint64();

    /* create the move performers */
    auto com_mover = pimc::CentreOfMassMovePerformer<double, NDIM> {n_timeslices, com_step_size};
    auto single_bead_mover = pimc::SingleBeadPositionMovePerformer<double, NDIM> {n_timeslices};

    /* create the file writers for the estimators */
    const auto output_dirpath = fs::path {"/home/a68ibrah/research/simulations/pimc-sim/playground/output"};
    auto kinetic_writer = estim::default_kinetic_writer<double>(output_dirpath);
    auto pair_potential_writer = estim::default_pair_potential_writer<double>(output_dirpath);
    auto rms_centroid_writer = estim::default_rms_centroid_distance_writer<double>(output_dirpath);
    auto abs_centroid_writer = estim::default_absolute_centroid_distance_writer<double>(output_dirpath);

    /* perform the simulation loop */
    for (std::size_t i_block {parser.first_block_index}; i_block < parser.last_block_index; ++i_block) {
        std::cout << "i_block = " << i_block << '\n';
        /* the number of passes is chosen such that the autocorrelation time between blocks is passed */
        for (std::size_t i_pass {0}; i_pass < parser.n_passes; ++i_pass) {
            /* perform COM move for each particle */
            for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
                com_mover(i_part, worldlines, prngw, interaction_handler, environment);

                for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
                    /* perform bead move on timeslice `i_tslice` of each particle */
                    single_bead_mover(i_part, i_tslice, worldlines, prngw, interaction_handler, environment);
                }
            }
        }

        if (i_block >= parser.n_equilibrium_blocks) {
            /* run estimators */
            const auto total_kinetic_energy = estim::total_primitive_kinetic_energy(worldlines, environment);
            const auto total_potential_energy = estim::total_pair_potential_energy(worldlines, pot, environment);
            const auto rms_centroid_dist = estim::rms_centroid_distance(worldlines, environment);
            const auto abs_centroid_dist = estim::absolute_centroid_distance(worldlines, environment);

            /* save estimators */
            kinetic_writer.write(i_block, total_kinetic_energy);
            pair_potential_writer.write(i_block, total_potential_energy);
            rms_centroid_writer.write(i_block, rms_centroid_dist);
            abs_centroid_writer.write(i_block, abs_centroid_dist);
        }
    }

    return 0;
}
