#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <iomanip>
#include <span>
#include <sstream>
#include <stdexcept>

#include "constants.hpp"

#include <coordinates/cartesian.hpp>

namespace coord
{
template <std::floating_point FP, std::size_t NDIM>
class BoxSides
{
public:
    // NOTE: there is no publicly available default constructor, because there
    // is no sensible default for the side lengths of the box
    BoxSides() = delete;

    template <typename... VarCoords>
    constexpr explicit BoxSides(VarCoords... coords)
        : m_coords {(coords)...}
    {
        static_assert(sizeof...(coords) == NDIM);
        ctr_check_all_entries_are_positive(m_coords);
    }

    constexpr explicit BoxSides(const Cartesian<FP, NDIM>& point)
        : m_coords {point.coordinates()}
    {
        ctr_check_all_entries_are_positive(m_coords);
    }

    constexpr auto coordinates() const noexcept -> const std::array<FP, NDIM>&
    {
        return m_coords;
    }

    constexpr auto operator[](std::size_t index) const noexcept -> FP
    {
        // non-modifying access coordinates with bounds checking on in debug mode only
        assert(index < NDIM);
        return m_coords[index];
    }

    constexpr auto at(std::size_t index) const -> FP
    {
        // non-modifying access coordinates with bounds checking always on
        if (index >= NDIM) {
            throw std::runtime_error {"Out of bounds access. Tried to access index " + std::to_string(index)};
        }

        return m_coords[index];
    }

    [[nodiscard]] auto as_string() const -> std::string
    {
        const auto prec = impl_coord::CARTESIAN_OSTREAM_PRECISION;
        std::stringstream coord_str;
        coord_str << "BoxSides(";
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

    constexpr void ctr_check_all_entries_are_positive(const std::span<const FP> container)
    {
        const auto is_nonpositive = [](auto x) { return x <= 0.0; };
        if (std::any_of(std::begin(container), std::end(container), is_nonpositive)) {
            throw std::runtime_error("All the box sides in a `BoxSides` instance must be positive.");
        }
    }
};

template <std::floating_point FP, std::size_t NDIM>
constexpr auto approx_eq(
    const BoxSides<FP, NDIM>& box0,
    const BoxSides<FP, NDIM>& box1,
    FP tolerance_sq = impl_coord::EPSILON_BOX_SEPARATION<FP>  //
) noexcept -> bool
{
    FP total_diff_sq {};
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        FP coord_diff = box0[i_dim] - box1[i_dim];
        total_diff_sq += (coord_diff * coord_diff);
    }

    return (total_diff_sq < tolerance_sq);
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto box_cutoff_distance(const BoxSides<FP, NDIM>& box) noexcept -> FP
{
    const auto& coords = box.coordinates();
    const auto minimum = std::min_element(std::begin(coords), std::end(coords));

    // there has to be at least one dimension; otherwise, the simulation couldn't even occur!
    const auto cutoff_distance = (*minimum) / FP {2.0};

    return cutoff_distance;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto box_cutoff_distance_squared(const BoxSides<FP, NDIM>& box) noexcept -> FP
{
    const auto dist = box_cutoff_distance<FP, NDIM>(box);
    return dist * dist;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto is_point_inside_box_around_origin(
    const Cartesian<FP, NDIM>& point,
    const BoxSides<FP, NDIM>& box
) noexcept -> bool
{
    const auto is_between = [](FP value, FP side_length)
    {
        const auto left = -side_length / FP {2.0};
        const auto right = side_length / FP {2.0};

        return left <= value && value < right;
    };

    for (std::size_t i {0}; i < NDIM; ++i) {
        if (!is_between(point[i], box[i])) {
            return false;
        }
    }

    return true;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto is_point_inside_box(
    const Cartesian<FP, NDIM>& point,
    const Cartesian<FP, NDIM>& origin,
    const BoxSides<FP, NDIM>& box
) noexcept -> bool
{
    return is_point_inside_box_around_origin(point - origin, box);
}

}  // namespace coord
