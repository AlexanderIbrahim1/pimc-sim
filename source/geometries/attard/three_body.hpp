#pragma once

/*
This header contains code to calculate the three pair distances between three
points in a lattice, using equations (4) and (5) in the paper by P. Attard.

P. Attard. "Simulation results for a fluid with the Axilrod-Teller triple dipole
potential", Phys. Rev. A, 45 (1992).
*/

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>

namespace impl_geom
{

template <std::floating_point FP>
struct ThreeBodySeparationCoordinates
{
    FP coord01;
    FP coord02;
    FP coord12;
};

template <std::floating_point FP>
auto cartesian_translation(FP x_i, FP x_j, FP box_side) noexcept -> FP
{
    // NOTE: this function was ported directly from the (working) Python version; it
    // seems that Python's built-in `round()` and `std::rint()` have the same behaviour,
    // even at half-integers
    const auto unrounded_shift = (x_i - x_j) / box_side;
    return box_side * std::rint(unrounded_shift);
}

template <std::floating_point FP>
auto separation_coordinates(FP x0, FP x1, FP x2, FP box_side) noexcept -> ThreeBodySeparationCoordinates<FP>
{
    const auto trans01 = cartesian_translation(x0, x1, box_side);
    const auto trans02 = cartesian_translation(x0, x2, box_side);

    const auto x01 = x0 - x1 - trans01;
    const auto x02 = x0 - x2 - trans02;
    const auto x12 = x1 - x2 + trans01 - trans02;

    // NOTE: using the following code to calculate x12 brings back the creation
    // of ambiguous triangles; so we can't do it this way
    // trans12 = _cartesian_translation(x1, x2, box_side)
    // x12 = x1 - x2 - trans12

    return {x01, x02, x12};
}

template <std::floating_point FP, std::size_t NDIM>
auto three_body_separation_points(
    const std::array<coord::Cartesian<FP, NDIM>, 3>& points,
    const coord::BoxSides<FP, NDIM>& box
) noexcept -> std::array<coord::Cartesian<FP, NDIM>, 3>
{
    const auto& p0 = points[0];
    const auto& p1 = points[1];
    const auto& p2 = points[2];

    auto separation01 = coord::Cartesian<FP, NDIM> {};
    auto separation02 = coord::Cartesian<FP, NDIM> {};
    auto separation12 = coord::Cartesian<FP, NDIM> {};

    for (std::size_t i_dim {0}; i_dim < NDIM; ++i_dim) {
        const auto [coord01, coord02, coord12] = separation_coordinates(p0[i_dim], p1[i_dim], p2[i_dim], box[i_dim]);
        separation01[i_dim] = coord01;
        separation02[i_dim] = coord02;
        separation12[i_dim] = coord12;
    }

    return {separation01, separation02, separation12};
}

}  // namespace impl_geom

namespace geom
{

template <std::floating_point FP, std::size_t NDIM>
auto three_body_attard_side_lengths_squared(
    const std::array<coord::Cartesian<FP, NDIM>, 3>& points,
    const coord::BoxSides<FP, NDIM>& box
) noexcept -> std::array<FP, 3>
{
    const auto separation_points = impl_geom::three_body_separation_points(points, box);

    const auto dist01_sq = coord::norm_squared(separation_points[0]);
    const auto dist02_sq = coord::norm_squared(separation_points[1]);
    const auto dist12_sq = coord::norm_squared(separation_points[2]);

    return {dist01_sq, dist02_sq, dist12_sq};
}

template <std::floating_point FP, std::size_t NDIM>
auto three_body_attard_side_lengths(
    const std::array<coord::Cartesian<FP, NDIM>, 3>& points,
    const coord::BoxSides<FP, NDIM>& box
) noexcept -> std::array<FP, 3>
{
    const auto [dist01_sq, dist02_sq, dist12_sq] = three_body_attard_side_lengths_squared(points, box);

    return {std::sqrt(dist01_sq), std::sqrt(dist02_sq), std::sqrt(dist12_sq)};
}

}  // namespace geom
