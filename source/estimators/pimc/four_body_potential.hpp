#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <concepts>
#include <span>
#include <vector>

#include <coordinates/attard/four_body.hpp>
#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <coordinates/periodic_shift.hpp>
#include <environment/environment.hpp>
#include <interactions/four_body/extrapolated_potential.hpp>
#include <interactions/four_body/potential_concepts.hpp>
#include <worldline/worldline.hpp>

namespace impl_estim
{

// MODIFIED
/*
    Calculate the total four-body potential energy for a collection of particles, assuming
    that all the particles have been translated in such a way that the particle at index `i0`
    is at the centre of the box.
*/
template <std::floating_point FP, std::size_t NDIM>
auto calculate_four_body_potential_energy_around_reference(
    std::size_t i0,
    interact::BufferedQuadrupletPotential<FP> auto& buffered_extrap_pot,
    std::span<const coord::Cartesian<FP, NDIM>> points,
    FP cutoff_distance_sq
) -> void
{
    const auto i1_final = points.size() - 2;
    const auto i2_final = points.size() - 1;
    const auto i3_final = points.size();

    for (std::size_t i1 {i0 + 1}; i1 < i1_final; ++i1) {
        const auto dist01_sq = coord::distance_squared(points[i0], points[i1]);
        if (dist01_sq > cutoff_distance_sq)
            continue;

        for (std::size_t i2 {i1 + 1}; i2 < i2_final; ++i2) {
            const auto dist02_sq = coord::distance_squared(points[i0], points[i2]);
            if (dist02_sq > cutoff_distance_sq)
                continue;

            const auto dist12_sq = coord::distance_squared(points[i1], points[i2]);
            if (dist12_sq > cutoff_distance_sq)
                continue;

            for (std::size_t i3 {i2 + 1}; i3 < i3_final; ++i3) {
                const auto dist03_sq = coord::distance_squared(points[i0], points[i3]);
                if (dist03_sq > cutoff_distance_sq)
                    continue;

                const auto dist13_sq = coord::distance_squared(points[i1], points[i3]);
                if (dist13_sq > cutoff_distance_sq)
                    continue;

                const auto dist23_sq = coord::distance_squared(points[i2], points[i3]);
                if (dist23_sq > cutoff_distance_sq)
                    continue;

                const auto dist01 = std::sqrt(dist01_sq);
                const auto dist02 = std::sqrt(dist02_sq);
                const auto dist03 = std::sqrt(dist03_sq);
                const auto dist12 = std::sqrt(dist12_sq);
                const auto dist13 = std::sqrt(dist13_sq);
                const auto dist23 = std::sqrt(dist23_sq);

                buffered_extrap_pot.add_sample({dist01, dist02, dist03, dist12, dist13, dist23});
            }
        }
    }
}

}  // namespace impl_estim

namespace estim
{

// MODIFIED
// NOTE: the other estimators assume that we are performing the estimate for the entire worldline at once
// - but for the four-body PES, we might have to break it up per timeslice!
template <std::floating_point FP, std::size_t NDIM>
auto timeslice_quadruplet_potential_energy(
    std::span<const coord::Cartesian<FP, NDIM>> points,
    interact::BufferedQuadrupletPotential<FP> auto& pot,
    const coord::BoxSides<FP, NDIM>& periodic_box,
    FP cutoff_distance
) -> FP
{
    if (points.size() < 4) {
        return FP {0.0};
    }

    const auto i0_final = points.size() - 3;
    const auto cutoff_distance_sq = cutoff_distance * cutoff_distance;

    for (std::size_t i0 {}; i0 < i0_final; ++i0) {
        const auto shifted_particles = coord::shift_points_together(i0, periodic_box, points);
        impl_estim::calculate_four_body_potential_energy_around_reference<FP, NDIM>(i0, pot, shifted_particles, cutoff_distance_sq);
    }

    return pot.extract_energy();
}

template <std::floating_point FP, std::size_t NDIM>
auto total_quadruplet_potential_energy_periodic(
    const worldline::Worldlines<FP, NDIM>& worldlines,
    interact::BufferedQuadrupletPotential<FP> auto& pot,
    const coord::BoxSides<FP, NDIM>& box,
    FP cutoff_distance
) -> FP
{
    auto total_pot = FP {};
    for (std::size_t i_tslice {0}; i_tslice < worldlines.n_timeslices(); ++i_tslice) {
        const auto timeslice = worldlines.timeslice(i_tslice);
        total_pot += timeslice_quadruplet_potential_energy(timeslice, pot, box, cutoff_distance);
    }

    return total_pot / static_cast<FP>(worldlines.n_timeslices());
}

}  // namespace estim
