#pragma once

#include <concepts>
#include <cstddef>
#include <vector>

#include <coordinates/coordinates.hpp>
#include <environment/environment.hpp>
#include <estimators/pimc/centroid.hpp>
#include <mathtools/histogram/histogram.hpp>
#include <worldline/worldline.hpp>

namespace estim
{

template <std::floating_point FP, std::size_t NDIM>
constexpr void update_centroid_radial_distribution_function_histogram(
    mathtools::Histogram<FP>& centroid_radial_dist_histo,
    const envir::Environment<FP>& environment,
    const coord::DistanceCalculator<FP, NDIM> auto& distance_calculator,
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines
)
{
    using Point = coord::Cartesian<FP, NDIM>;

    const auto n_particles = environment.n_particles();

    auto centroids = std::vector<Point> {};
    centroids.reserve(n_particles);
    for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
        auto centroid = create_centroid(worldlines, i_part);
        centroids.emplace_back(centroid);
    }

    for (std::size_t ip0 {0}; ip0 < n_particles - 1; ++ip0) {
        const auto p0 = centroids[ip0];
        for (std::size_t ip1 {ip0 + 1}; ip1 < n_particles; ++ip1) {
            const auto p1 = centroids[ip1];

            const auto distance = distance_calculator(p0, p1);
            centroid_radial_dist_histo.add(distance);
        }
    }
}

}  // namespace estim
