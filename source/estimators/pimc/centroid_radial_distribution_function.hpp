#pragma once

#include <concepts>
#include <cstddef>

#include <coordinates/measure_concepts.hpp>
#include <mathtools/histogram/histogram.hpp>
#include <worldline/worldline.hpp>

namespace estim
{

template <std::floating_point FP, std::size_t NDIM>
void update_centroid_radial_distribution_function_histogram(
    mathtools::Histogram<FP>& centroid_radial_dist_histo,
    const coord::DistanceCalculator<FP, NDIM> auto& distance_calculator,
    const worldline::Worldlines<FP, NDIM>& worldlines
)
{
    const auto n_particles = worldlines.n_worldlines();
    const auto centroids = worldline::calculate_all_centroids(worldlines);

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
