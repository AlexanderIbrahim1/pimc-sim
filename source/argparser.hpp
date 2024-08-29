#pragma once

#include <concepts>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>

#include <tomlplusplus/toml.hpp>

namespace argparse
{

template <typename T>
auto cast_toml_to(const toml::table& table, std::string_view name) -> T
{
    const auto maybe_value = table[name].value<T>();
    if (!maybe_value) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to parse '" << name << "' from the toml stream.\n";
        throw std::runtime_error {err_msg.str()};
    }

    return *maybe_value;
}

template <>
auto cast_toml_to(const toml::table& table, std::string_view name) -> std::filesystem::path
{
    const auto maybe_value = table[name].value<std::string>();
    if (!maybe_value) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to parse '" << name << "' from the toml stream.\n";
        throw std::runtime_error {err_msg.str()};
    }

    return std::filesystem::path{*maybe_value};
}

/*
    There are too many types of exceptions to handle from parsing all the data properly from the toml file,
    so to simplify things for the user I decided to catch all the choices and allow the user to just check
    a flag and maybe check the error message.
*/
template <std::floating_point FP>
class ArgParser
{
public:
    explicit ArgParser(std::stringstream& toml_stream)
    {
        parse_helper_(toml_stream);
    }

    explicit ArgParser(const std::filesystem::path& toml_filepath)
    {
        auto toml_stream = std::ifstream {toml_filepath};
        if (!toml_stream.is_open()) {
            auto err_msg = std::stringstream {};
            err_msg << "ERROR: Unable to open the toml file for parsing: '";
            err_msg << toml_filepath;
            err_msg << '\n';

            parse_success_flag_ = false;
            error_message_ = err_msg.str();
        } else {
            parse_helper_(toml_stream);
        }
    }

    constexpr auto is_valid() const noexcept -> bool
    {
        return parse_success_flag_;
    }

    constexpr auto error_message() const noexcept -> std::string
    {
        return error_message_;
    }

    std::filesystem::path abs_output_dirpath {};
    std::size_t first_block_index {};
    std::size_t last_block_index {};
    std::size_t n_equilibrium_blocks {};
    std::size_t n_passes {};
    std::size_t n_timeslices {};
    FP centre_of_mass_step_size {};
    std::size_t bisection_level {};
    FP bisection_ratio {};
    FP density {};
    FP temperature {};
    std::tuple<std::size_t, std::size_t, std::size_t> n_unit_cells {};
    std::filesystem::path abs_two_body_filepath {};
    std::filesystem::path abs_three_body_filepath {};
    std::filesystem::path abs_four_body_filepath {};

private:
    bool parse_success_flag_ {};
    std::string error_message_ {};

    void parse_helper_(std::istream& toml_stream) {
        try {
            const auto table = toml::parse(toml_stream);

            abs_output_dirpath = cast_toml_to<std::filesystem::path>(table, "abs_output_dirpath");
            first_block_index = cast_toml_to<std::size_t>(table, "first_block_index");
            last_block_index = cast_toml_to<std::size_t>(table, "last_block_index");
            n_equilibrium_blocks = cast_toml_to<std::size_t>(table, "n_equilibrium_blocks");
            n_passes = cast_toml_to<std::size_t>(table, "n_passes");
            n_timeslices = cast_toml_to<std::size_t>(table, "n_timeslices");
            centre_of_mass_step_size = cast_toml_to<FP>(table, "centre_of_mass_step_size");
            bisection_level = cast_toml_to<std::size_t>(table, "bisection_level");
            bisection_ratio = cast_toml_to<FP>(table, "bisection_ratio");
            density = cast_toml_to<FP>(table, "density");
            temperature = cast_toml_to<FP>(table, "temperature");
            std::get<0>(n_unit_cells) = cast_toml_to<std::size_t>(table, "n_cells_dim0");
            std::get<1>(n_unit_cells) = cast_toml_to<std::size_t>(table, "n_cells_dim1");
            std::get<2>(n_unit_cells) = cast_toml_to<std::size_t>(table, "n_cells_dim2");
            abs_two_body_filepath = cast_toml_to<std::filesystem::path>(table, "abs_two_body_filepath");
            abs_three_body_filepath = cast_toml_to<std::filesystem::path>(table, "abs_three_body_filepath");
            abs_four_body_filepath = cast_toml_to<std::filesystem::path>(table, "abs_four_body_filepath");

            parse_success_flag_ = true;
        }
        catch (const toml::parse_error& err) {
            parse_success_flag_ = false;
            error_message_ = err.what();
        }
        catch (const std::runtime_error& err) {
            parse_success_flag_ = false;
            error_message_ = err.what();
        }
    }
};

}  // namespace argparse
