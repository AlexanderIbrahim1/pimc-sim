#pragma once

#include <cmath>
#include <concepts>
#include <numeric>
#include <stdexcept>
#include <vector>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <worldline/worldline.hpp>

namespace estim
{

template <std::floating_point FP, std::size_t NDIM>
constexpr auto rms_centroid_distance(
    const worldline::Worldlines<FP, NDIM>& worldlines
) -> FP
{
    const auto n_particles = worldlines.n_particles();
    const auto n_timeslices = worldlines.n_timeslices();

    auto total = FP {0.0};

    // NOTE: this sum is not done over contiguous elements, but this isn't part of the hot loop,
    //       so I'm not too concerned about the performance issues here
    for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
        auto centroid = worldline::calculate_centroid(worldlines, i_part);

        for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
            const auto point = worldlines.get(i_tslice, i_part);
            total += coord::distance_squared(point, centroid);
        }
    }

    // in the case of calculating the squared distances; the "mean" in RMS is taken over the timeslices,
    // so we put the number of timeslices inside the square root
    return std::sqrt(total / static_cast<FP>(n_timeslices)) / static_cast<FP>(n_particles);
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto absolute_centroid_distance(
    const worldline::Worldlines<FP, NDIM>& worldlines
) -> FP
{
    const auto n_particles = worldlines.n_particles();
    const auto n_timeslices = worldlines.n_timeslices();

    auto total = FP {0.0};

    // NOTE: this sum is not done over contiguous elements, but this isn't part of the hot loop,
    //       so I'm not too concerned about the performance issues here
    for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
        auto centroid = worldline::calculate_centroid(worldlines, i_part);

        for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
            const auto point = worldlines.get(i_tslice, i_part);
            total += coord::distance(point, centroid);
        }
    }

    // in the case of calcualting the absolute distances; we don't follow the RMS equation, and so the
    // number of timeslices is just in the denominator (no square root to take anyways)
    return total / static_cast<FP>(n_timeslices * n_particles);
}

}  // namespace estim
