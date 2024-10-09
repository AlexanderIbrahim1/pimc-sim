#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>

#include <common/writers/writer_utils.hpp>

namespace geom_utils
{

template <std::floating_point FP>
void check_lattice_constant_is_positive(FP lattice_constant)
{
    if (lattice_constant <= FP {0.0}) {
        const auto precision = common::writers::DEFAULT_WRITER_SINGLE_VALUE_PRECISION;

        auto err_msg = std::stringstream {};
        err_msg << "The lattice constant must be positive. Found: ";
        err_msg << std::scientific << std::setprecision(precision) << lattice_constant << '\n';
        throw std::runtime_error(err_msg.str());
    }
}

template <std::size_t NDIM>
void check_unit_cell_translations_are_positive(const std::array<std::size_t, NDIM>& translations)
{
    const auto less_than_one = [](std::size_t x) { return x < 1; };
    if (std::any_of(std::begin(translations), std::end(translations), less_than_one)) {
        throw std::runtime_error("The unit cell translations must all be positive.");
    }
}

}  // namespace geom_utils
