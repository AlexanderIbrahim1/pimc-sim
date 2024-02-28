#pragma once

#include <concepts>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

#include <common/common_utils.hpp>
#include <common/writers/double_value_writer.hpp>

namespace pimc
{

namespace writer_utils
{
constexpr auto DEFAULT_BISECTION_MULTIBEAD_POSITION_MOVE_SUCCESS_WRITER =
    std::string_view {"bisection_multibead_position_move_accept.dat"};
constexpr auto DEFAULT_SINGLE_BEAD_POSITION_MOVE_SUCCESS_WRITER =
    std::string_view {"single_bead_position_move_accept.dat"};
constexpr auto DEFAULT_CENTRE_OF_MASS_POSITION_MOVE_SUCCESS_WRITER =
    std::string_view {"centre_of_mass_position_move_accept.dat"};

}  // namespace writer_utils

}  // namespace pimc

namespace pimc
{

auto default_bisection_multibead_position_move_success_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t>
{
    const auto filepath = output_dirpath / pimc::writer_utils::DEFAULT_BISECTION_MULTIBEAD_POSITION_MOVE_SUCCESS_WRITER;
    const auto header =
        std::string {"# number of accepted and rejected moves in the bisection multibead position move\n"};

    return common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t> {filepath, header};
}

auto default_single_bead_position_move_success_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t>
{
    const auto filepath = output_dirpath / pimc::writer_utils::DEFAULT_SINGLE_BEAD_POSITION_MOVE_SUCCESS_WRITER;
    const auto header = std::string {"# number of accepted and rejected moves in the single bead position move\n"};

    return common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t> {filepath, header};
}

auto default_centre_of_mass_position_move_success_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t>
{
    const auto filepath = output_dirpath / pimc::writer_utils::DEFAULT_CENTRE_OF_MASS_POSITION_MOVE_SUCCESS_WRITER;
    const auto header = std::string {"# number of accepted and rejected moves in the centre of mass position move\n"};

    return common::writers::DoubleValueBlockWriter<std::uint64_t, std::uint64_t> {filepath, header};
}

}  // namespace pimc
