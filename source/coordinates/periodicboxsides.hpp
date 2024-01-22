#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <sstream>
#include <stdexcept>

#include "constants.hpp"

namespace coord
{
template <std::floating_point FP, std::size_t NDIM>
class PeriodicBoxSides
{
public:
    // NOTE: there is no publicly available default constructor, because there
    // is no sensible default for the side lengths of the box
    PeriodicBoxSides() = delete;

    template <typename... VarCoords>
    explicit PeriodicBoxSides(VarCoords... coords)
        : m_coords {(coords)...}
    {
        static_assert(sizeof...(coords) == NDIM);

        const auto is_nonpositive = [](FP x) { return x <= 0.0; };
        if (std::any_of(m_coords.cbegin(), m_coords.cend(), is_nonpositive)) {
            throw std::runtime_error("All the box sides in a `PeriodicBoxSides` instance must be positive.");
        }
    }

    constexpr auto coordinates() const noexcept -> const std::array<FP, NDIM>&
    {
        return m_coords;
    }

    constexpr auto operator[](std::size_t index) const noexcept -> FP
    {
        // modifying access coordinates with bounds checking on in debug mode only
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

    [[nodiscard]] auto as_string() const -> std::string
    {
        const auto prec = CARTESIAN_OSTREAM_PRECISION;
        std::stringstream coord_str;
        coord_str << "PeriodicBoxSides(";
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

private:
    std::array<FP, NDIM> m_coords;
};

// NOTE: write a unit test for this!
template <std::floating_point FP, std::size_t NDIM>
constexpr auto approx_eq(
    const PeriodicBoxSides<FP, NDIM>& box0,
    const PeriodicBoxSides<FP, NDIM>& box1,
    FP tolerance_sq = EPSILON_BOX_SEPARATION<FP>  //
) noexcept -> bool
{
    FP total_diff_sq {};
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        FP coord_diff = box0[i_dim] - box1[i_dim];
        total_diff_sq += (coord_diff * coord_diff);
    }

    return (total_diff_sq < tolerance_sq);
}

}  // namespace coord
