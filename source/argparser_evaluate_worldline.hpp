#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <common/toml_utils.hpp>
#include <tomlplusplus/toml.hpp>

namespace argparse
{

/*
    There are too many types of exceptions to handle from parsing all the data properly from the toml file,
    so to simplify things for the user I decided to catch all the choices and allow the user to just check
    a flag and maybe check the error message.
*/
template <std::floating_point FP>
class EvaluateWorldlineArgParser
{
public:
    explicit EvaluateWorldlineArgParser(std::stringstream& toml_stream)
    {
        parse_helper_(toml_stream);
    }

    explicit EvaluateWorldlineArgParser(const std::filesystem::path& toml_filepath)
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
    std::filesystem::path abs_worldlines_dirpath {};
    std::vector<std::size_t> block_indices {};
    FP density {};
    std::tuple<std::size_t, std::size_t, std::size_t> n_unit_cells {};
    std::filesystem::path abs_two_body_filepath {};
    std::filesystem::path abs_three_body_filepath {};
    std::filesystem::path abs_four_body_filepath {};
    bool evaluate_two_body {};
    bool evaluate_three_body {};
    bool evaluate_four_body {};

private:
    bool parse_success_flag_ {};
    std::string error_message_ {};

    void parse_helper_(std::istream& toml_stream)
    {
        try {
            using common::io::cast_toml_to;

            const auto table = toml::parse(toml_stream);

            abs_output_dirpath = cast_toml_to<std::filesystem::path>(table, "abs_output_dirpath");
            abs_worldlines_dirpath = cast_toml_to<std::filesystem::path>(table, "abs_worldlines_dirpath");
            parse_block_indices_(table);
            density = cast_toml_to<FP>(table, "density");
            std::get<0>(n_unit_cells) = cast_toml_to<std::size_t>(table, "n_cells_dim0");
            std::get<1>(n_unit_cells) = cast_toml_to<std::size_t>(table, "n_cells_dim1");
            std::get<2>(n_unit_cells) = cast_toml_to<std::size_t>(table, "n_cells_dim2");
            abs_two_body_filepath = cast_toml_to<std::filesystem::path>(table, "abs_two_body_filepath");
            abs_three_body_filepath = cast_toml_to<std::filesystem::path>(table, "abs_three_body_filepath");
            abs_four_body_filepath = cast_toml_to<std::filesystem::path>(table, "abs_four_body_filepath");
            evaluate_two_body = cast_toml_to<bool>(table, "evaluate_two_body");
            evaluate_three_body = cast_toml_to<bool>(table, "evaluate_three_body");
            evaluate_four_body = cast_toml_to<bool>(table, "evaluate_four_body");

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

    void parse_block_indices_(const toml::table& table)
    {
        if (auto block_indices_array = table["block_indices"].as_array()) {
            for (auto& element : *block_indices_array) {
                if (element.is_integer()) {
                    const auto value = element.value<std::int64_t>();
                    if (!value) {
                        throw std::runtime_error {"ERROR: unable to parse integer in 'block_indices'."};
                    }

                    if (*value < 0) {
                        throw std::runtime_error {"ERROR: found a negative block index."};
                    }

                    const auto index = static_cast<std::size_t>(*value);
                    block_indices.push_back(index);
                }
                else {
                    throw std::runtime_error {"ERROR: found non-integer element in 'block_indices'."};
                }
            }
        }
        else {
            throw std::runtime_error {"ERROR: 'block_indices' not found, or not an array in the file."};
        }
    }
};

}  // namespace argparse
