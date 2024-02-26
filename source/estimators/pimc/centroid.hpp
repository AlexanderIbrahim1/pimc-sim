#pragma once

#include <cmath>
#include <concepts>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <worldline/worldline.hpp>

namespace estim
{

template <std::floating_point FP, std::size_t NDIM, bool IsSquaredDistance>
constexpr auto mean_centroid_base_(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const envir::Environment<FP>& environment
) -> FP
{
    using Point = coord::Cartesian<FP, NDIM>;

    if (worldlines.empty()) {
        return FP {0.0};
    }

    const auto n_particles = environment.n_particles();
    const auto n_timeslices = environment.n_timeslices();

    const auto create_centroid = [&worldlines, &n_timeslices](std::size_t i_part) -> Point
    {
        auto centroid = Point::origin();
        for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
            centroid += worldlines[i_tslice][i_part];
        }
        centroid /= static_cast<FP>(n_timeslices);

        return centroid;
    };

    auto total = FP {0.0};

    // NOTE: this sum is not done over contiguous elements, but this isn't part of the hot loop,
    //       so I'm not too concerned about the performance issues here
    for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
        auto centroid = create_centroid(i_part);

        for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
            const auto point = worldlines[i_tslice][i_part];
            if constexpr (IsSquaredDistance) {
                total += coord::distance_squared(point, centroid);
            }
            else {
                total += coord::distance(point, centroid);
            }
        }
    }

    if constexpr (IsSquaredDistance) {
        // in the case of calculating the squared distances; the "mean" in RMS is taken over the timeslices,
        // so we put the number of timeslices inside the square root
        return std::sqrt(total / static_cast<FP>(n_timeslices)) / static_cast<FP>(n_particles);
    }
    else {
        // in the case of calcualting the absolute distances; we don't follow the RMS equation, and so the
        // number of timeslices is just in the denominator (no square root to take anyways)
        return total / static_cast<FP>(n_timeslices * n_particles);
    }
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto rms_centroid_distance(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const envir::Environment<FP>& environment
) -> FP
{
    constexpr auto is_squared_distance = true;
    return mean_centroid_base_<FP, NDIM, is_squared_distance>(worldlines, environment);
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto absolute_centroid_distance(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const envir::Environment<FP>& environment
) -> FP
{
    constexpr auto is_squared_distance = false;
    return mean_centroid_base_<FP, NDIM, is_squared_distance>(worldlines, environment);
}

}  // namespace estim
