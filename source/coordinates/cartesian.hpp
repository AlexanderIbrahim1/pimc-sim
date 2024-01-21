#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "constants.hpp"

namespace coord
{
template <std::floating_point FP, std::size_t NDIM>
class Cartesian
{
private:
    std::array<FP, NDIM> m_coords;

public:
    template <typename... VarCoords>
    explicit Cartesian(VarCoords... coords)
        : m_coords {(coords)...}
    {
        static_assert(sizeof...(coords) == NDIM);
    }

    explicit Cartesian(std::array<FP, NDIM> coords)
        : m_coords {coords}
    {}

    Cartesian()
        : m_coords {{}}
    {}

    constexpr auto coordinates() const -> std::array<FP, NDIM>
    {
        return m_coords;
    }

    constexpr void shift_coord(std::size_t index, FP value)
    {
        assert(index < NDIM);
        m_coords[index] += value;
    }

    constexpr void shift_coord_checked(std::size_t index, FP value)
    {
        // version of `shift_coord` with bounds checking always on
        if (index >= NDIM) {
            throw std::runtime_error("Out of bounds access. Tried to access index " + std::to_string(index));
        }
        m_coords[index] += value;
    }

    constexpr auto operator[](std::size_t index) const -> FP
    {
        // non-modifying access coordinates with bounds checking on during debug mode only
        assert(index < NDIM);
        return m_coords[index];
    }

    constexpr auto operator[](std::size_t index) -> FP&
    {
        // modifying access coordinates with bounds checking on during debug mode only
        assert(index < NDIM);
        return m_coords[index];
    }

    constexpr auto at(std::size_t index) const -> FP
    {
        // non-modifying access coordinates with bounds checking always on
        if (index >= NDIM) {
            throw std::runtime_error("Out of bounds access. Tried to access index " + std::to_string(index));
        }

        return m_coords[index];
    }

    constexpr void at(std::size_t index, FP new_coord)
    {
        // modifying access coordinates with bounds checking always on
        if (index >= NDIM) {
            throw std::runtime_error("Out of bounds access. Tried to access index " + std::to_string(index));
        }

        m_coords[index] = new_coord;
    }

    [[nodiscard]] auto as_string() const -> std::string
    {
        const auto prec = CARTESIAN_OSTREAM_PRECISION;
        std::stringstream coord_str;
        coord_str << "(";
        for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
            const auto value = m_coords[i_dim];

            // replicates the "space or negative sign" formatting from Python
            if (value >= FP {0.0}) {
                coord_str << ' ';
            }

            coord_str << std::fixed << std::setprecision(prec) << value;

            if (i_dim < NDIM - 1) {
                coord_str << ", ";
            }
            else {
                coord_str << ")";
            }
        }

        return coord_str.str();
    }

    constexpr auto operator+=(const Cartesian<FP, NDIM>& rhs) -> Cartesian<FP, NDIM>&
    {
        for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
            m_coords[i_dim] = m_coords[i_dim] + rhs[i_dim];
        }

        return *this;
    }

    constexpr auto operator-=(const Cartesian<FP, NDIM>& rhs) -> Cartesian<FP, NDIM>&
    {
        for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
            m_coords[i_dim] = m_coords[i_dim] - rhs[i_dim];
        }

        return *this;
    }

    constexpr auto operator/=(FP other) -> Cartesian<FP, NDIM>&
    {
#ifndef NDEBUG
        // I can't think of a good one-size-fits-all way to prevent zero-division
        // errors; what I've decided on, is to perform a run-time check when debug
        // mode is activated, and to omit this check when NDEBUG is present.
        //
        // it would be a *bad* idea (or at least very limiting) to check for equality
        // between 'other' and 0.0; I opt not to do that here
        if (std::fabs(other) < EPSILON_CARTESIAN_ZERO_DIVIDE<FP>) {
            throw std::runtime_error("Division by zero exception.");
        }
#endif

        for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
            m_coords[i_dim] = m_coords[i_dim] / other;
        }

        return *this;
    }

    constexpr auto operator*=(FP other) -> Cartesian<FP, NDIM>&
    {
        for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
            m_coords[i_dim] = m_coords[i_dim] * other;
        }

        return *this;
    }

    constexpr auto operator-() const -> Cartesian<FP, NDIM>
    {
        auto new_coordinates = std::array<FP, NDIM> {};
        for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
            new_coordinates[i_dim] = -m_coords[i_dim];
        }

        return Cartesian<FP, NDIM> {std::move(new_coordinates)};
    }

    constexpr auto operator+() const -> Cartesian<FP, NDIM>
    {
        return Cartesian<FP, NDIM>(m_coords);
    }

    constexpr static auto origin() -> Cartesian<FP, NDIM>
    {
        return Cartesian();
    }
};

// NOTICE: in all five of the following operator overloading functions, `lhs` is passed
// in as a copy; so this function modifies and returns a copy of `lhs`, creating a new
// instance

template <std::floating_point FP, std::size_t NDIM>
constexpr auto operator+(Cartesian<FP, NDIM> lhs, const Cartesian<FP, NDIM>& rhs) -> Cartesian<FP, NDIM>
{
    lhs += rhs;
    return lhs;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto operator-(Cartesian<FP, NDIM> lhs, const Cartesian<FP, NDIM>& rhs) -> Cartesian<FP, NDIM>
{
    lhs -= rhs;
    return lhs;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto operator*(Cartesian<FP, NDIM> lhs, FP rhs) -> Cartesian<FP, NDIM>
{
    lhs *= rhs;
    return lhs;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto operator*(FP lhs, Cartesian<FP, NDIM> rhs) -> Cartesian<FP, NDIM>
{
    rhs *= lhs;
    return rhs;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto operator/(Cartesian<FP, NDIM> lhs, FP rhs) -> Cartesian<FP, NDIM>
{
    lhs /= rhs;
    return lhs;
}

}  // namespace coord
