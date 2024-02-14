#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include <tomlplusplus/toml.hpp>

#include <argparser.hpp>
#include <constants/constants.hpp>
#include <coordinates/box_sides.hpp>
#include <coordinates/coordinates.hpp>
#include <environment/environment.hpp>
#include <geometries/bravais.hpp>
#include <geometries/lattice.hpp>
#include <geometries/lattice_type.hpp>
#include <geometries/unit_cell_translations.hpp>
#include <interactions/handlers/periodic_full_pair_interaction_handler.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
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

auto main() -> int
{
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

    if (!parser.is_valid()) {
        std::cout << "PARSER DID NOT PARSE PROPERLY\n";
        std::exit(EXIT_FAILURE);
    }

    /* create the lattice positions and the periodic box */
    const auto lattice_type = geom::LatticeType::HCP;
    const auto lattice_constant = geom::density_to_lattice_constant(parser.density, lattice_type);
    const auto hcp_unit_cell = geom::conventional_hcp_unit_cell(lattice_constant);
    const auto hcp_unit_cell_box = geom::unit_cell_box_sides(hcp_unit_cell);
    const auto lattice_box_translations = geom::UnitCellTranslations<NDIM> {1, 1, 1};
    const auto minimage_box = geom::lattice_box(hcp_unit_cell_box, lattice_box_translations);

    const auto lattice_site_positions = geom::lattice_particle_positions(hcp_unit_cell, lattice_box_translations);

    /* create the worldlines from the lattice site positions */

    /*
        Parameters for the Lennard-Jones potential are taken from paragraph 3 of page 354
        of `Eur. Phys. J. D 56, 353–358 (2010)`. Original units are in Kelvin and Angstroms,
        converted to wavenumbers and angstroms.
    */
    const auto pot = interact::LennardJonesPotential {23.77, 2.96};

    /* create the interaction handler */
    const auto interaction_handler = interact::PeriodicFullPairInteractionHandler {pot, minimage_box};

    /* create the environment object */
    const auto h2_mass = constants::H2_MASS_IN_AMU<double>;
    const auto environment =
        envir::create_finite_temperature_environment(parser.temperature, parser.n_timeslices, h2_mass);

    /* create the move performers */
    auto com_mover =
        pimc::CentreOfMassMovePerformer<double, NDIM> {parser.n_timeslices, parser.centre_of_mass_step_size};
    auto single_bead_mover = pimc::SingleBeadPositionMovePerformer<double, NDIM> {parser.n_timeslices};
    /* create the objects needed to properly use the move performers */

    /* create the PRNG; save the seed (or set it?) */
    auto prngw = rng::RandomNumberGeneratorWrapper<std::mt19937>::from_random_uint64();

    /* perform the simulation loop */
    for (std::size_t i_block {parser.first_block_index}; i_block < parser.last_block_index; ++i_block) {
        /* the number of passes is chosen such that the autocorrelation time between blocks is passed */
        for (std::size_t i_pass {0}; i_pass < parser.n_passes; ++i_pass) {
            for (std::size_t i_tslice {0}; i_tslice < parser.n_timeslices; ++i_tslice) {
                if (i_tslice == 0) {
                    /* perform COM move for each particle */
                }

                /* perform bead move on timeslice `i_tslice` of each particle */
            }
        }

        if (i_block >= parser.n_equilibrium_blocks) {
            /* run estimators */
            /* save estimators */
        }
    }

    const auto point = coord::Cartesian<double, 2> {1.0, 2.0};
    std::cout << point.as_string() << '\n';

    return 0;
}
