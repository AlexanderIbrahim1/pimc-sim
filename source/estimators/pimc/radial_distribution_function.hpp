#pragma once

#include <concepts>
#include <cstddef>
#include <vector>

#include <coordinates/coordinates.hpp>
#include <mathtools/histogram/histogram.hpp>
#include <worldline/worldline.hpp>

namespace estim
{

template <std::floating_point FP, std::size_t NDIM>
constexpr void update_radial_distribution_function_histogram(
    mathtools::Histogram<FP>& radial_dist_histo,
    const coord::DistanceCalculator<FP, NDIM> auto& distance_calculator,
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines
)
{
    for (const auto& worldline : worldlines) {
        const auto& points = worldline.points();

        for (std::size_t ip0 {0}; ip0 < points.size() - 1; ++ip0) {
            const auto p0 = points[ip0];
            for (std::size_t ip1 {ip0 + 1}; ip1 < points.size(); ++ip1) {
                const auto p1 = points[ip1];

                const auto distance = distance_calculator(p0, p1);
                radial_dist_histo.add(distance);
            }
        }
    }
}

}  // namespace estim
