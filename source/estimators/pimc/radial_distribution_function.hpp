#pragma once

#include <concepts>
#include <cstddef>

#include <coordinates/measure_concepts.hpp>
#include <mathtools/histogram/histogram.hpp>
#include <worldline/worldline.hpp>

namespace estim
{

template <std::floating_point FP, std::size_t NDIM>
void update_radial_distribution_function_histogram(
    mathtools::Histogram<FP>& radial_dist_histo,
    const coord::DistanceCalculator<FP, NDIM> auto& distance_calculator,
    const worldline::Worldlines<FP, NDIM>& worldlines
)
{
    for (std::size_t i_tslice {0}; i_tslice < worldlines.n_timeslices(); ++i_tslice)
    {
        const auto timeslice = worldlines.timeslice(i_tslice);
        for (std::size_t ip0 {0}; ip0 < timeslice.size() - 1; ++ip0) {
            const auto p0 = timeslice[ip0];
            for (std::size_t ip1 {ip0 + 1}; ip1 < timeslice.size(); ++ip1) {
                const auto p1 = timeslice[ip1];

                const auto distance = distance_calculator(p0, p1);
                radial_dist_histo.add(distance);
            }
        }
    }
}

}  // namespace estim
