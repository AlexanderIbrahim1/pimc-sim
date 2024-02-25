#pragma once

#include <concepts>
#include <filesystem>
#include <string>

#include <estimators/estim_utils.hpp>
#include <estimators/writers/single_value_writer.hpp>

namespace estim
{

template <std::floating_point FP>
constexpr auto default_kinetic_writer(const std::filesystem::path& output_dirpath) -> SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / estim_utils::DEFAULT_KINETIC_OUTPUT_FILENAME;
    const auto header = std::string {"# total kinetic energy in wavenumbers\n"};

    return SingleValueBlockWriter<FP> {filepath, header};
}

template <std::floating_point FP>
constexpr auto default_pair_potential_writer(const std::filesystem::path& output_dirpath) -> SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / estim_utils::DEFAULT_PAIR_POTENTIAL_OUTPUT_FILENAME;
    const auto header = std::string {"# total pair potential energy in wavenumbers\n"};

    return SingleValueBlockWriter<FP> {filepath, header};
}

template <std::floating_point FP>
constexpr auto default_centroid_writer(const std::filesystem::path& output_dirpath) -> SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / estim_utils::DEFAULT_CENTROID_OUTPUT_FILENAME;
    const auto header = std::string {"# mean distance from centroid to bead in angstroms\n"};

    return SingleValueBlockWriter<FP> {filepath, header};
}

}  // namespace estim
