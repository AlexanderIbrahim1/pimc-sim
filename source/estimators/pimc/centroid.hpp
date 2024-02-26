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

template <std::floating_point FP, std::size_t NDIM>
constexpr auto mean_centroid_squared_distance(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const envir::Environment<FP>& environment
) -> FP
{
    if (worldlines.empty()) {
        return FP {0.0};
    }

    auto total_dist_squared = FP {};

    for (const auto& wline : worldlines) {
        const auto& points = wline.points();

        if (points.size() == 0 || points.size() == 1) {
            continue;
        }

        const auto centroid = coord::calculate_centroid<FP, NDIM>(points);
        for (auto point : points) {
            total_dist_squared += coord::distance_squared(point, centroid);
        }
    }

    const auto n_beads = environment.n_particles() * environment.n_timeslices();

    return std::sqrt(total_dist_squared) / static_cast<FP>(n_beads);
}

}  // namespace estim
