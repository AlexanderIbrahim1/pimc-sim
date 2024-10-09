#pragma once

#include <concepts>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

#include <common/writers/double_value_writer.hpp>

namespace pimc
{

static auto default_bisection_multibead_position_move_success_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t>
{
    const auto filepath = output_dirpath / "bisection_multibead_position_move_accept.dat";
    const auto header =
        std::string {"# number of accepted and rejected moves in the bisection multibead position move\n"};

    return common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t> {filepath, header};
}

static auto default_single_bead_position_move_success_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t>
{
    const auto filepath = output_dirpath / "single_bead_position_move_accept.dat";
    const auto header = std::string {"# number of accepted and rejected moves in the single bead position move\n"};

    return common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t> {filepath, header};
}

static auto default_centre_of_mass_position_move_success_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t>
{
    const auto filepath = output_dirpath / "centre_of_mass_position_move_accept.dat";
    const auto header = std::string {"# number of accepted and rejected moves in the centre of mass position move\n"};

    return common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t> {filepath, header};
}

template <std::floating_point FP>
static auto default_centre_of_mass_position_move_step_size_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::SingleValueBlockWriter<FP>
{
    const auto filepath = output_dirpath / "centre_of_mass_step_size.dat";
    const auto header = std::string {"# step size parameter for the centre of mass position move\n"};

    return common::writers::SingleValueBlockWriter<FP> {filepath, header};
}

template <std::floating_point FP>
static auto default_bisection_multibead_position_move_info_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::DoubleValueBlockWriter<FP, std::uint64_t>
{
    const auto filepath = output_dirpath / "bisection_multibead_position_move_info.dat";
    const auto header =
        std::string {"# [upper level fraction] and [lower level] for the bisection multibead position move\n"};

    return common::writers::DoubleValueBlockWriter<FP, std::uint64_t> {filepath, header};
}

}  // namespace pimc
