#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include <tomlplusplus/toml.hpp>

#include <argparser.hpp>
#include <coordinates/box_sides.hpp>
#include <coordinates/coordinates.hpp>
#include <geometries/lattice_type.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
#include <rng/distributions.hpp>
#include <rng/generator.hpp>

/*
PLAN for main

What might I read in later?
- information needed to create the potentials
- nearest neighbour info
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

    const auto box = coord::BoxSides<double, NDIM> {1.0, 2.0, 3.0};

    /* create the pair potential */

    /* create the interaction handler */

    /* create the environment object */

    /* create the move performers */
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

    // auto norm_dist = rng::NormalDistribution<double> {};
    // static_assert(rng::PRNGWrapper<rng::RandomNumberGeneratorWrapper<std::mt19937>>);

    // std::cout << "SEED = " << prngw.seed() << '\n';

    // for (std::size_t i {0}; i < 20; ++i) {
    //     std::cout << norm_dist.normal_01(prngw) << '\n';
    // }

    return 0;
}
