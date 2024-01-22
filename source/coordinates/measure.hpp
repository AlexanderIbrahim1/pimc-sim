#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>

#include "constants.hpp"
#include "coordinates.hpp"
#include "periodicboxsides.hpp"

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
    const PeriodicBoxSides<FP, NDIM>& box
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
    const PeriodicBoxSides<FP, NDIM>& box  //
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
    const PeriodicBoxSides<FP, NDIM>& box  //
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
auto norm_periodic(const Cartesian<FP, NDIM>& point, const PeriodicBoxSides<FP, NDIM>& box) noexcept -> FP
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
    const PeriodicBoxSides<FP, NDIM>& box,
    FP tolerance_sq = EPSILON_APPROX_EQ_SEPARATION_SQUARED<FP>  //
) noexcept -> bool
{
    const auto separation_dist_sq = distance_squared_periodic(point0, point1, box);
    return separation_dist_sq < tolerance_sq;
}

template <typename SizedContainer1, typename SizedContainer2>
auto approx_eq_containers(const SizedContainer1& cont1, const SizedContainer2& cont2) noexcept -> bool
{
    if (cont1.size() != cont2.size()) {
        return false;
    }

    for (auto it1 = std::begin(cont1), it2 = std::begin(cont2); it1 < std::end(cont1); ++it1, ++it2) {
        if (!approx_eq(*it1, *it2)) {
            return false;
        }
    }

    return true;
}

}  // namespace coord
