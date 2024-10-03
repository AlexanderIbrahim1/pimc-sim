#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/constants.hpp>

// NOTE: any function involving the use of `sqrt()` cannot be `constexpr` until C++26
// - the fact that it compiles on GCC is apparently a bug, and it won't work on other compilers

namespace coord
{

template <std::floating_point FP, std::size_t NDIM>
constexpr auto distance_squared(const Cartesian<FP, NDIM>& point0, const Cartesian<FP, NDIM>& point1) noexcept -> FP
{
    FP dist_sq = 0.0;
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        auto separation = point0[i_dim] - point1[i_dim];
        dist_sq += separation * separation;
    }

    return dist_sq;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto distance_squared_periodic(
    const Cartesian<FP, NDIM>& point0,
    const Cartesian<FP, NDIM>& point1,
    const BoxSides<FP, NDIM>& box  //
) noexcept -> FP
{
    FP dist_sq = 0.0;
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        auto separation = point0[i_dim] - point1[i_dim];

        const auto n_boxshifts = std::rint(separation / box[i_dim]);
        separation -= (box[i_dim] * n_boxshifts);

        dist_sq += separation * separation;
    }

    return dist_sq;
}

template <std::floating_point FP, std::size_t NDIM>
auto distance(const Cartesian<FP, NDIM>& point0, const Cartesian<FP, NDIM>& point1) noexcept -> FP
{
    return std::sqrt(distance_squared(point0, point1));
}

template <std::floating_point FP, std::size_t NDIM>
auto distance_periodic(
    const Cartesian<FP, NDIM>& point0,
    const Cartesian<FP, NDIM>& point1,
    const BoxSides<FP, NDIM>& box  //
) noexcept -> FP
{
    return std::sqrt(distance_squared_periodic(point0, point1, box));
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto norm_squared(const Cartesian<FP, NDIM>& point) noexcept -> FP
{
    FP norm_sq = 0.0;
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        auto coord = point[i_dim];
        norm_sq += coord * coord;
    }

    return norm_sq;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto norm_squared_periodic(
    const Cartesian<FP, NDIM>& point,
    const BoxSides<FP, NDIM>& box  //
) noexcept -> FP
{
    FP norm_sq = 0.0;
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        auto coord = point[i_dim];

        const auto n_boxshifts = std::rint(coord / box[i_dim]);
        coord -= (box[i_dim] * n_boxshifts);

        norm_sq += coord * coord;
    }

    return norm_sq;
}

template <std::floating_point FP, std::size_t NDIM>
auto norm(const Cartesian<FP, NDIM>& point) noexcept -> FP
{
    return std::sqrt(norm_squared(point));
}

template <std::floating_point FP, std::size_t NDIM>
auto norm_periodic(const Cartesian<FP, NDIM>& point, const BoxSides<FP, NDIM>& box) noexcept -> FP
{
    return std::sqrt(norm_squared_periodic(point, box));
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto approx_eq(
    const Cartesian<FP, NDIM>& point0,
    const Cartesian<FP, NDIM>& point1,
    FP tolerance_sq = EPSILON_APPROX_EQ_SEPARATION_SQUARED<FP>  //
) noexcept -> bool
{
    const auto separation_dist_sq = distance_squared(point0, point1);
    return separation_dist_sq < tolerance_sq;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto approx_eq_periodic(
    const Cartesian<FP, NDIM>& point0,
    const Cartesian<FP, NDIM>& point1,
    const BoxSides<FP, NDIM>& box,
    FP tolerance_sq = EPSILON_APPROX_EQ_SEPARATION_SQUARED<FP>  //
) noexcept -> bool
{
    const auto separation_dist_sq = distance_squared_periodic(point0, point1, box);
    return separation_dist_sq < tolerance_sq;
}

template <std::floating_point FP, std::size_t NDIM>
auto approx_eq_containers(
    const std::span<const Cartesian<FP, NDIM>> span1,
    const std::span<const Cartesian<FP, NDIM>> span2,
    FP tolerance_sq = EPSILON_APPROX_EQ_SEPARATION_SQUARED<FP>  //
) noexcept -> bool
{
    if (span1.size() != span2.size()) {
        return false;
    }

    for (auto it1 = std::begin(span1), it2 = std::begin(span2); it1 < std::end(span1); ++it1, ++it2) {
        if (!approx_eq(*it1, *it2, tolerance_sq)) {
            return false;
        }
    }

    return true;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto calculate_centroid(const std::span<const coord::Cartesian<FP, NDIM>> points)
    -> coord::Cartesian<FP, NDIM>
{
    using Point = coord::Cartesian<FP, NDIM>;

    if (points.size() == 0) {
        throw std::runtime_error("Cannot calculate centroid of empty sequence of points.");
    }

    const auto total_point = std::accumulate(std::begin(points), std::end(points), Point::origin());
    return total_point / static_cast<FP>(points.size());
}

}  // namespace coord
