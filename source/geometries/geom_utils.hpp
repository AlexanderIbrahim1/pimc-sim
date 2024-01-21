#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <format>
#include <stdexcept>

namespace geom_utils
{

template <std::floating_point FP>
constexpr void check_lattice_constant_is_positive(FP lattice_constant)
{
    if (lattice_constant <= FP {0.0}) {
        const auto msg = std::format("The lattice constant must be positive. Found: {}\n", lattice_constant);
        throw std::runtime_error(msg);
    }
}

template <std::size_t NDIM>
constexpr void check_unit_cell_translations_are_positive(const std::array<std::size_t, NDIM>& translations)
{
    const auto less_than_one = [](std::size_t x) { return x < 1; };
    if (std::any_of(std::begin(translations), std::end(translations), less_than_one)) {
        throw std::runtime_error(std::format("The unit cell translations must all be positive."));
    }
}

}  // namespace geom_utils
