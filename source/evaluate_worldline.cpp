#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include <torch/script.h>

#include <common/writers/single_value_writer.hpp>
#include <coordinates/box_sides.hpp>
#include <estimators/pimc/three_body_potential.hpp>
#include <estimators/pimc/two_body_potential.hpp>
#include <estimators/pimc/four_body_potential.hpp>
#include <estimators/writers/default_writers.hpp>
#include <interactions/four_body/published_potential.hpp>
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
    const auto parser = argparse::EvaluateWorldlineArgParser<float> {toml_input_filename};
    if (!parser.is_valid()) {
        std::cout << "ERROR: argument parser did not parse properly\n";
        std::cout << parser.error_message() << '\n';
        std::exit(EXIT_FAILURE);
    }

    const auto output_dirpath = parser.abs_output_dirpath;
    const auto [n_particles, minimage_box, lattice_site_positions] = build_hcp_lattice_structure(parser.density, parser.n_unit_cells);
    const auto fourbody_cutoff = coord::box_cutoff_distance(minimage_box);

    // clang-format off
    using ReturnType2B = decltype(fsh_potential<float>(minimage_box, parser.abs_two_body_filepath));
    const auto pot2b = [&]() -> std::optional<ReturnType2B> {
        if (parser.evaluate_two_body) {
            return std::make_optional(fsh_potential<float>(minimage_box, parser.abs_two_body_filepath));
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

    const long int buffer_size = 1024;
    using ReturnType4B = decltype(interact::get_published_buffered_four_body_potential<NDIM, interact::PermutationTransformerFlag::EXACT>(parser.abs_four_body_filepath, buffer_size));
    auto pot4b = [&]() -> std::optional<ReturnType4B> {
        if (parser.evaluate_four_body) {
            auto pot4b_ = interact::get_published_buffered_four_body_potential<NDIM, interact::PermutationTransformerFlag::EXACT>(parser.abs_four_body_filepath, buffer_size);
            return std::make_optional(std::move(pot4b_));
        } else {
            return std::nullopt;
        }
    }();

    /* create the file writers for the estimators */
    const auto pair_potential_writer = estim::default_pair_potential_writer<float>(output_dirpath);
    const auto triplet_potential_writer = estim::default_triplet_potential_writer<float>(output_dirpath);
    const auto quadruplet_potential_writer = estim::default_quadruplet_potential_writer<float>(output_dirpath);

    /* the worldline writer is needed to create the filepaths to the worldline files */
    auto worldline_writer = worldline::WorldlineWriter<float, NDIM> {parser.abs_worldlines_dirpath};

    // const auto quadruplet_header = std::string {"# total quadruplet potential energy per timeslice in wavenumbers\n"};
    // const auto quadruplet_filename = estim::writers::DEFAULT_QUADRUPLET_POTENTIAL_OUTPUT_FILENAME;
    // const auto quadruplet_potential_writer = common::writers::SingleValueBlockWriter<float> {output_dirpath / quadruplet_filename, quadruplet_header};

    /* create the timer and the corresponding writer to keep track of how long each block takes */
    auto timer = sim::Timer {};
    const auto timer_writer = sim::default_timer_writer(output_dirpath);

    for (auto block_index : parser.block_indices) {
        /* create the worldlines */
        const auto worldlines = [&]()
        {
            const auto worldline_filepath = worldline_writer.output_filepath(block_index);
            return worldline::read_worldlines<float, NDIM>(worldline_filepath);
        }();

        timer.start();

        // clang-format off
        /* run estimators */
        if (pot2b) {
            const auto total_pair_potential_energy = estim::total_pair_potential_energy_periodic(worldlines, pot2b.value());
            pair_potential_writer.write(block_index, total_pair_potential_energy);
        }

        if (pot3b) {
            const auto total_triplet_potential_energy = estim::total_triplet_potential_energy_periodic(worldlines, pot3b.value());
            triplet_potential_writer.write(block_index, total_triplet_potential_energy);
        }

        if (pot4b) {
            const auto total_quadruplet_potential_energy = estim::total_quadruplet_potential_energy_periodic(worldlines, pot4b.value(), minimage_box, fourbody_cutoff);
            quadruplet_potential_writer.write(block_index, total_quadruplet_potential_energy);
        }
        // clang-format on

        const auto duration = timer.duration_since_last_start();
        timer_writer.write(block_index, duration.seconds, duration.milliseconds, duration.microseconds);
    }

    return 0;
}
