#pragma once

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <tomlplusplus/toml.hpp>

namespace common_utils
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

}   // namespace common_utils
