#pragma once

#include <sstream>
#include <stdexcept>
#include <string_view>

#include <tomlplusplus/toml.hpp>

namespace argparse
{

// What do I read in?
// - first block id
// - last block id
// - number of equilibrium blocks
// - number of passes
// - number of slices
// - the move hyperparameters
//   - com step size
//   - bisection level

template <typename T>
constexpr auto cast_toml_to(const toml::table& table, std::string_view name) -> T
{
    const auto maybe_value = table[name].value<T>();
    if (!maybe_value) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to parse '" << name << "' from the toml stream.\n";
        throw std::runtime_error {err_msg.str()};
    }

    return *maybe_value;
}

class ArgParser
{
public:
    explicit ArgParser(std::stringstream& toml_stream)
    {
        try {
            const auto table = toml::parse(toml_stream);

            first_block_index = cast_toml_to<std::size_t>(table, "first_block_index");
            last_block_index = cast_toml_to<std::size_t>(table, "last_block_index");
            n_equilibrium_blocks = cast_toml_to<std::size_t>(table, "n_equilibrium_blocks");
            n_passes = cast_toml_to<std::size_t>(table, "n_passes");
            n_timeslices = cast_toml_to<std::size_t>(table, "n_timeslices");
            centre_of_mass_step_size = cast_toml_to<double>(table, "centre_of_mass_step_size");
            bisection_level = cast_toml_to<std::size_t>(table, "bisection_level");
            bisection_ratio = cast_toml_to<double>(table, "bisection_ratio");
            density = cast_toml_to<double>(table, "density");
            temperature = cast_toml_to<double>(table, "temperature");

            parse_success_flag_ = true;
        }
        catch (const toml::parse_error& err) {
            parse_success_flag_ = false;
        }
        catch (const std::runtime_error& err) {
            parse_success_flag_ = false;
        }
    }

    constexpr auto is_valid() const noexcept -> bool
    {
        return parse_success_flag_;
    }

    std::size_t first_block_index {};
    std::size_t last_block_index {};
    std::size_t n_equilibrium_blocks {};
    std::size_t n_passes {};
    std::size_t n_timeslices {};
    double centre_of_mass_step_size {};
    std::size_t bisection_level {};
    double bisection_ratio {};
    double density {};
    double temperature {};

private:
    bool parse_success_flag_ {};
};

}  // namespace argparse
