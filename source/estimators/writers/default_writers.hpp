#pragma once

#include <concepts>
#include <filesystem>
#include <string>
#include <string_view>

#include <common/common_utils.hpp>
#include <common/writers/single_value_writer.hpp>

namespace estim
{

namespace writers
{
constexpr auto DEFAULT_KINETIC_OUTPUT_FILENAME = std::string_view {"kinetic.dat"};
constexpr auto DEFAULT_PAIR_POTENTIAL_OUTPUT_FILENAME = std::string_view {"pair_potential.dat"};
constexpr auto DEFAULT_TRIPLET_POTENTIAL_OUTPUT_FILENAME = std::string_view {"triplet_potential.dat"};
constexpr auto DEFAULT_QUADRUPLET_POTENTIAL_OUTPUT_FILENAME = std::string_view {"quadruplet_potential.dat"};
constexpr auto DEFAULT_RMS_CENTROID_DISTANCE_OUTPUT_FILENAME = std::string_view {"rms_centroid_distance.dat"};
constexpr auto DEFAULT_ABSOLUTE_CENTROID_DISTANCE_OUTPUT_FILENAME = std::string_view {"absolute_centroid_distance.dat"};
}  // namespace writers

}  // namespace estim

namespace estim
{

template <std::floating_point FP>
auto default_kinetic_writer(const std::filesystem::path& output_dirpath) -> common::writers::SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / estim::writers::DEFAULT_KINETIC_OUTPUT_FILENAME;
    const auto header = std::string {"# total kinetic energy in wavenumbers\n"};

    return common::writers::SingleValueBlockWriter<FP> {filepath, header};
}

template <std::floating_point FP>
auto default_pair_potential_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / estim::writers::DEFAULT_PAIR_POTENTIAL_OUTPUT_FILENAME;
    const auto header = std::string {"# total pair potential energy in wavenumbers\n"};

    return common::writers::SingleValueBlockWriter<FP> {filepath, header};
}

template <std::floating_point FP>
auto default_triplet_potential_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / estim::writers::DEFAULT_TRIPLET_POTENTIAL_OUTPUT_FILENAME;
    const auto header = std::string {"# total triplet potential energy in wavenumbers\n"};

    return common::writers::SingleValueBlockWriter<FP> {filepath, header};
}

template <std::floating_point FP>
auto default_quadruplet_potential_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / estim::writers::DEFAULT_QUADRUPLET_POTENTIAL_OUTPUT_FILENAME;
    const auto header = std::string {"# total quadruplet potential energy in wavenumbers\n"};

    return common::writers::SingleValueBlockWriter<FP> {filepath, header};
}

template <std::floating_point FP>
auto default_rms_centroid_distance_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / estim::writers::DEFAULT_RMS_CENTROID_DISTANCE_OUTPUT_FILENAME;
    const auto header =
        std::string {"# rms distance from bead to centroid, averaged over all particles, in angstroms\n"};

    return common::writers::SingleValueBlockWriter<FP> {filepath, header};
}

template <std::floating_point FP>
auto default_absolute_centroid_distance_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / estim::writers::DEFAULT_ABSOLUTE_CENTROID_DISTANCE_OUTPUT_FILENAME;
    const auto header =
        std::string {"# absolute distance from bead to centroid, averaged over all particles, in angstroms\n"};

    return common::writers::SingleValueBlockWriter<FP> {filepath, header};
}

}  // namespace estim
