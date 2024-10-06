#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

// #include <torch/script.h>

#include <common/writers/writer_utils.hpp>
#include <coordinates/box_sides.hpp>
#include <coordinates/measure_wrappers.hpp>
#include <environment/environment.hpp>
#include <estimators/pimc/two_body_potential.hpp>
#include <estimators/pimc/three_body_potential.hpp>
// #include <estimators/pimc/four_body_potential.hpp>
#include <estimators/writers/default_writers.hpp>
// #include <interactions/four_body/published_potential.hpp>
#include <pimc/writers/default_writers.hpp>
#include <simulation/timer.hpp>
#include <worldline/writers/read_worldlines.hpp>
#include <worldline/writers/worldline_writer.hpp>

#include <argparser_evaluate_worldline.hpp>
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
    const auto parser = argparse::EvaluateWorldlineArgParser<double> {toml_input_filename};
    if (!parser.is_valid()) {
        std::cout << "ERROR: argument parser did not parse properly\n";
        std::cout << parser.error_message() << '\n';
        std::exit(EXIT_FAILURE);
    }

    const auto output_dirpath = parser.abs_output_dirpath;
    const auto temperature = 1.0;  // temperature doesn't matter for worldline evaluation
    const auto n_timeslices = parser.n_timeslices;
    const auto block_index = parser.block_index;
    const auto [n_particles, minimage_box, lattice_site_positions] = build_hcp_lattice_structure(parser.density, parser.n_unit_cells);

    const auto periodic_distance_squared_calculator = coord::PeriodicDistanceSquaredMeasureWrapper<double, NDIM> {minimage_box};
    const auto pair_cutoff_distance = coord::box_cutoff_distance(minimage_box);

    /* create the worldlines */
    auto worldlines = [&]() {
        auto worldline_writer = worldline::WorldlineWriter<double, NDIM> {output_dirpath};
        const auto worldline_filepath = worldline_writer.output_filepath(block_index);
        return worldline::read_worldlines<double, NDIM>(worldline_filepath);
    }();

    // clang-format off
    using ReturnType2B = decltype(fsh_potential<double>(minimage_box, parser.abs_two_body_filepath));
    const auto pot2b = [&]() -> std::optional<ReturnType2B> {
        if (parser.evaluate_two_body) {
            return std::make_optional(fsh_potential<double>(minimage_box, parser.abs_two_body_filepath));
        } else {
            return std::nullopt;
        }
    }();

    using ReturnType3B = decltype(threebodyparah2_potential(minimage_box, parser.abs_three_body_filepath));
    const auto pot3b = [&]() -> std::optional<ReturnType3B> {
        if (parser.evaluate_three_body) {
            return std::make_optional(threebodyparah2_potential(minimage_box, parser.abs_three_body_filepath));
        } else {
            return std::nullopt;
        }
    }();

    /*
    using ReturnType4B = decltype(interact::get_published_buffered_four_body_potential<NDIM, interact::PermutationTransformerFlag::EXACT>(parser.abs_four_body_filepath, buffer_size));
    const auto pot4b = [&]() -> std::optional<ReturnType4B> {
        if (parser.evaluate_four_body) {
            const long int buffer_size = 1024;
            auto pot4b = interact::get_published_buffered_four_body_potential<NDIM, interact::PermutationTransformerFlag::EXACT>(parser.abs_four_body_filepath, buffer_size);
            return std::make_optional(std::move(pot4b));
        } else {
            return std::nullopt;
        }
    }();
    */

    /* create the environment object */
    const auto h2_mass = constants::H2_MASS_IN_AMU<double>;
    const auto environment = envir::create_environment(temperature, h2_mass, n_timeslices, n_particles);

    /* create the file writers for the estimators */
    const auto pair_potential_writer = estim::default_pair_potential_writer<double>(output_dirpath);
    const auto triplet_potential_writer = estim::default_triplet_potential_writer<double>(output_dirpath);
    // const auto quadruplet_potential_writer = estim::default_quadruplet_potential_writer<float>(output_dirpath);

    /* create the timer and the corresponding writer to keep track of how long each block takes */
    auto timer = sim::Timer {};
    const auto timer_writer = sim::default_timer_writer(output_dirpath);

    timer.start();
    // clang-format off

    /* run estimators */
    if (pot2b) {
        // const auto total_pair_potential_energy = estim::total_pair_potential_energy_periodic(worldlines, pot2b.value(), environment);
        const auto total_pair_potential_energy = estim::total_pair_potential_energy_periodic_with_centroid_cutoff(worldlines, pot2b.value(), environment, periodic_distance_squared_calculator, pair_cutoff_distance);
        pair_potential_writer.write(block_index, total_pair_potential_energy);
    }

    if (pot3b) {
        const auto total_triplet_potential_energy = estim::total_triplet_potential_energy_periodic(worldlines, pot3b.value(), environment);
        triplet_potential_writer.write(block_index, total_triplet_potential_energy);
    }

    /*
    if (pot4b) {
        const auto cutoff = coord::box_cutoff_distance(minimage_box);
        const auto total_quadruplet_potential_energy = estim::calculate_total_four_body_potential_energy_via_shifting(worldlines, pot4b.value(), environment, minimage_box, cutoff);
        quadruplet_potential_writer.write(block_index, total_quadruplet_potential_energy);
    }
    */
    // clang-format on

    const auto duration = timer.duration_since_last_start();
    timer_writer.write(block_index, duration.seconds, duration.milliseconds, duration.microseconds);

    return 0;
}
