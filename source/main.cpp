#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include <tomlplusplus/toml.hpp>

#include <argparser.hpp>
#include <coordinates/coordinates.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
#include <rng/distributions.hpp>
#include <rng/generator.hpp>

/*
PLAN for main

What might I read in later?
- information needed to create the potentials
- nearest neighbour info
*/

template <interact::PairDistancePotential Potential>
auto takes_pointwise_pair_distance_potential(Potential pot)
{
    std::cout << pot(2.0) << '\n';
}

auto main() -> int
{
    //            first_block_index = cast_toml_to<std::size_t>(table, "first_block_index");
    //            last_block_index = cast_toml_to<std::size_t>(table, "last_block_index");
    //            n_equilibrium_blocks = cast_toml_to<std::size_t>(table, "n_equilibrium_blocks");
    //            n_passes = cast_toml_to<std::size_t>(table, "n_passes");
    //            n_timeslices = cast_toml_to<std::size_t>(table, "n_timeslices");
    //            centre_of_mass_step_size = cast_toml_to<double>(table, "centre_of_mass_step_size");

    const auto toml_input = std::string_view {R"(
        first_block_index = 0
        last_block_index = 200
        n_equilibrium_blocks = 20
        n_passes = 10
        n_timeslices = 32
        centre_of_mass_step_size = 0.3
        bisection_level = 3
        bisection_ratio = 0.4
    )"};

    auto toml_stream = std::stringstream {std::string {toml_input}};
    const auto parser = argparse::ArgParser {toml_stream};

    if (!parser.is_valid()) {
        std::cout << "PARSER DID NOT PARSE PROPERLY\n";
    }
    else {
        std::cout << "PARSER WORKED!!!\n";
        std::cout << "first_block_index = " << parser.first_block_index << '\n';
        std::cout << "last_block_index = " << parser.last_block_index << '\n';
        std::cout << "n_equilibrium_blocks = " << parser.n_equilibrium_blocks << '\n';
        std::cout << "n_passes = " << parser.n_passes << '\n';
        std::cout << "n_timeslices = " << parser.n_timeslices << '\n';
        std::cout << "centre_of_mass_step_size = " << parser.centre_of_mass_step_size << '\n';
        std::cout << "bisection_level = " << parser.bisection_level << '\n';
        std::cout << "bisection_ratio = " << parser.bisection_ratio << '\n';
    }

    const auto point = coord::Cartesian<double, 2> {1.0, 2.0};
    std::cout << point.as_string() << '\n';

    const auto potential = interact::LennardJonesPotential<double> {1.0, 1.0};
    takes_pointwise_pair_distance_potential(potential);

    auto prngw = rng::RandomNumberGeneratorWrapper<std::mt19937>::from_random_uint64();
    auto norm_dist = rng::NormalDistribution<double> {};

    static_assert(rng::PRNGWrapper<rng::RandomNumberGeneratorWrapper<std::mt19937>>);

    std::cout << "SEED = " << prngw.seed() << '\n';

    for (std::size_t i {0}; i < 20; ++i) {
        std::cout << norm_dist.normal_01(prngw) << '\n';
    }

    return 0;
}
