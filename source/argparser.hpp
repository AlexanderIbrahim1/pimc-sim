#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>

#include <rng/prng_state.hpp>

#include <common/toml_utils.hpp>
#include <tomlplusplus/toml.hpp>

namespace argparse
{

constexpr auto SEED_STRING_FLAG_OPTIONS = std::array<std::string_view, 2> {"RANDOM", "TIME_SINCE_EPOCH"};

constexpr auto map_seed_string_flag_options(std::string_view flag) -> rng::RandomSeedFlag
{
    using RSF = rng::RandomSeedFlag;
    if (flag == SEED_STRING_FLAG_OPTIONS[0]) {
        return RSF::RANDOM;
    }
    else if (flag == SEED_STRING_FLAG_OPTIONS[1]) {
        return RSF::TIME_SINCE_EPOCH;
    }
    else {
        throw std::logic_error("impossible mapping for random seed flag");
    }
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
        }
        else {
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
    std::variant<rng::RandomSeedFlag, std::uint64_t> initial_seed_state;

private:
    bool parse_success_flag_ {};
    std::string error_message_ {};

    void parse_helper_(std::istream& toml_stream)
    {
        try {
            using common_utils::cast_toml_to;

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

            parse_seed_(table);

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

    void parse_seed_(const toml::table& table)
    {
        const auto maybe_uint64t = table["initial_seed"].value<std::uint64_t>();
        if (maybe_uint64t) {
            initial_seed_state = *maybe_uint64t;
            return;
        }

        const auto maybe_string = table["initial_seed"].value<std::string>();
        if (maybe_string) {
            const auto seed_string = *maybe_string;

            const auto found = std::any_of(
                std::begin(SEED_STRING_FLAG_OPTIONS),
                std::end(SEED_STRING_FLAG_OPTIONS),
                [&seed_string](std::string_view sv) { return seed_string == sv; }
            );

            if (!found) {
                parse_seed_error_message_();
            }

            initial_seed_state = map_seed_string_flag_options(seed_string);
            return;
        }

        parse_seed_error_message_();
    }

    void parse_seed_error_message_()
    {
        auto err_msg = std::stringstream {};
        err_msg << "'initial_seed' must be an integer that fits in a 64-bit unsigned integer, or a string.\n";
        err_msg << "If 'initial_seed' is provided as a string, it must have one of the following values:\n";
        for (auto option : SEED_STRING_FLAG_OPTIONS) {
            err_msg << option << '\n';
        }
        throw std::runtime_error {err_msg.str()};
    }
};

}  // namespace argparse
