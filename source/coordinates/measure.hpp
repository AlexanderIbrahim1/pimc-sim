#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "constants.hpp"
#include "coordinates.hpp"
#include "periodicboxsides.hpp"

namespace coord
{
template <typename FP, std::size_t NDIM>
FP distance_squared(const Cartesian<FP, NDIM>& point0, const Cartesian<FP, NDIM>& point1) {
    FP dist_sq = 0.0;
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        auto separation = point0[i_dim] - point1[i_dim];
        dist_sq += separation * separation;
    }

    return dist_sq;
}

template <typename FP, std::size_t NDIM>
FP distance_squared_periodic(const Cartesian<FP, NDIM>& point0,
                             const Cartesian<FP, NDIM>& point1,
                             const PeriodicBoxSides<FP, NDIM>& box) {
    FP dist_sq = 0.0;
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        auto separation = point0[i_dim] - point1[i_dim];

        const auto n_boxshifts = std::rint(separation / box[i_dim]);
        separation -= (box[i_dim] * n_boxshifts);

        dist_sq += separation * separation;
    }

    return dist_sq;
}

template <typename FP, std::size_t NDIM>
FP distance(const Cartesian<FP, NDIM>& point0, const Cartesian<FP, NDIM>& point1) {
    return std::sqrt(distance_squared(point0, point1));
}

template <typename FP, std::size_t NDIM>
FP distance_periodic(const Cartesian<FP, NDIM>& point0,
                     const Cartesian<FP, NDIM>& point1,
                     const PeriodicBoxSides<FP, NDIM>& box) {
    return std::sqrt(distance_squared_periodic(point0, point1, box));
}

template <typename FP, std::size_t NDIM>
FP norm_squared(const Cartesian<FP, NDIM>& point) {
    FP norm_sq = 0.0;
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        auto coord = point[i_dim];
        norm_sq += coord * coord;
    }

    return norm_sq;
}

template <typename FP, std::size_t NDIM>
FP norm_squared_periodic(const Cartesian<FP, NDIM>& point, const PeriodicBoxSides<FP, NDIM>& box) {
    FP norm_sq = 0.0;
    for (std::size_t i_dim = 0; i_dim < NDIM; ++i_dim) {
        auto coord = point[i_dim];

        const auto n_boxshifts = std::rint(coord / box[i_dim]);
        coord -= (box[i_dim] * n_boxshifts);

        norm_sq += coord * coord;
    }

    return norm_sq;
}

template <typename FP, std::size_t NDIM>
FP norm(const Cartesian<FP, NDIM>& point) {
    return std::sqrt(norm_squared(point));
}

template <typename FP, std::size_t NDIM>
FP norm_periodic(const Cartesian<FP, NDIM>& point, const PeriodicBoxSides<FP, NDIM>& box) {
    return std::sqrt(norm_squared_periodic(point, box));
}

template <typename FP, std::size_t NDIM>
bool approx_eq(const Cartesian<FP, NDIM>& point0,
               const Cartesian<FP, NDIM>& point1,
               FP tolerance_sq = EPSILON_APPROX_EQ_SEPARATION_SQUARED<FP>) {
    const auto separation_dist_sq = distance_squared(point0, point1);
    return separation_dist_sq < tolerance_sq;
}

template <typename FP, std::size_t NDIM>
bool approx_eq_periodic(const Cartesian<FP, NDIM>& point0,
                        const Cartesian<FP, NDIM>& point1,
                        const PeriodicBoxSides<FP, NDIM>& box,
                        FP tolerance_sq = EPSILON_APPROX_EQ_SEPARATION_SQUARED<FP>) {
    const auto separation_dist_sq = distance_squared_periodic(point0, point1, box);
    return separation_dist_sq < tolerance_sq;
}

}  // namespace coord
