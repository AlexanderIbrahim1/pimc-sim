#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
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
template <std::floating_point FP, std::size_t NDIM>
auto calculate_four_body_potential_energy_preshift_inner_loops(
    std::size_t i0,
    interact::BufferedQuadrupletPotential<FP> auto& buffered_extrap_pot,
    const std::vector<coord::Cartesian<FP, NDIM>>& points,
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

                buffered_extrap_pot.add_sample(coord::FourBodySideLengths<FP> {
                    dist01, dist02, dist03, dist12, dist13, dist23});
            }
        }
    }
}

// MODIFIED
// NOTE: the other estimators assume that we are performing the estimate for the entire worldline at once
// - but for the four-body PES, we might have to break it up per timeslice!
template <std::floating_point FP, std::size_t NDIM>
auto calculate_total_four_body_potential_energy_via_shifting(
    const worldline::Worldline<FP, NDIM>& worldline,
    interact::BufferedQuadrupletPotential<FP> auto& buffered_extrap_pot,
    const envir::Environment<FP>& environment,
    const coord::BoxSides<FP, NDIM>& periodic_box,
    FP cutoff_distance
) -> FP
{
    if (worldline.size() < 4) {
        return FP {0.0};
    }

    const auto i0_final = worldline.size() - 3;
    const auto cutoff_distance_sq = cutoff_distance * cutoff_distance;

    for (std::size_t i0 {}; i0 < i0_final; ++i0) {
        // NOTE: possible optimization here by reusing the same vector memory each time we
        //       calculate `shifted_particles`; revisit if performance becomes an issue
        const auto shifted_particles = coord::shift_points_together(i0, periodic_box, worldline.points());
        calculate_four_body_potential_energy_preshift_inner_loops(
            i0, buffered_extrap_pot, shifted_particles, cutoff_distance_sq
        );
    }

    return buffered_extrap_pot.extract_energy();
}

}  // namespace impl_estim

namespace estim
{

template <std::floating_point FP, std::size_t NDIM>
auto calculate_total_four_body_potential_energy_via_shifting(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    interact::BufferedQuadrupletPotential<FP> auto& buffered_extrap_pot,
    const envir::Environment<FP>& environment,
    const coord::BoxSides<FP, NDIM>& periodic_box,
    FP cutoff_distance
) -> FP
{
    auto total_pot = FP {};
    for (const auto& worldline : worldlines) {
        total_pot += impl_estim::calculate_total_four_body_potential_energy_via_shifting(
            worldline, buffered_extrap_pot, environment, periodic_box, cutoff_distance
        );
    }

    return total_pot / static_cast<FP>(environment.n_timeslices());
}

// MODIFIED
// template <std::floating_point FP, std::size_t NDIM>
// auto calculate_four_body_potential_energy_preshift_felt_by(
//     std::size_t i0,
//     interact::BufferedExtrapolatedPotential<FP, NDIM>& buffered_extrap_pot,
//     const std::vector<coord::Cartesian<FP, NDIM>>& particles,
//     const coord::BoxSides<FP, NDIM>& periodic_box,
//     FP cutoff_distance
// ) -> FP
// {
//     if (particles.size() < 4) {
//         return FloatingType {0.0};
//     }
//
//     const auto shifted_particles = [&]()
//     {
//         auto particles_ = coord::shift_points_together(i0, periodic_box, particles);
//         std::iter_swap(particles_.begin(), particles_.begin() + i0);
//         return particles_;
//     }();
//
//     const auto cutoff_distance_sq = cutoff_distance * cutoff_distance;
//     const auto four_body_energy = calculate_four_body_potential_energy_preshift_inner_loops(
//         0, buffered_extrap_pot, shifted_particles, cutoff_distance_sq
//     );
//
//     return four_body_energy;
// }

// MODIFIED
// template <std::floating_point FP, std::size_t NDIM>
// auto calculate_total_four_body_potential_energy_fast(
//     interact::BufferedExtrapolatedPotential<FP, NDIM>& buffered_extrap_pot,
//     const std::vector<coord::Cartesian<FP, NDIM>>& particles,
//     const coord::BoxSides<FP, NDIM>& periodic_box,
//     FP cutoff_distance
// ) -> FP
// {
//     using ERT = attard::EarlyResultType;
//
//     if (particles.size() < 4) {
//         return FloatingType {0.0};
//     }
//
//     std::size_t i0_final = particles.size() - 3;
//     std::size_t i1_final = particles.size() - 2;
//     std::size_t i2_final = particles.size() - 1;
//     std::size_t i3_final = particles.size();
//
//     const auto cutoff_distance_sq = cutoff_distance * cutoff_distance;
//
//     for (std::size_t i0 {}; i0 < i0_final; ++i0)
//         for (std::size_t i1 {i0 + 1}; i1 < i1_final; ++i1)
//             for (std::size_t i2 {i1 + 1}; i2 < i2_final; ++i2)
//                 for (std::size_t i3 {i2 + 1}; i3 < i3_final; ++i3) {
//                     const auto attard_result = coord::four_body_attard_side_lengths_early(
//                         particles[i0], particles[i1], particles[i2], particles[i3], periodic_box, cutoff_distance_sq
//                     );
//
//                     // depending on the condition, possibly switch to the end condition for an inner loop
//                     // to jump to the start of the wrapping loops
//                     switch (attard_result.type) {
//                         case ERT::valid : {
//                             buffered_extrap_pot.add_sample(attard_result.sides);
//                             break;
//                         }
//                         case ERT::next1 : {
//                             i2 = i2_final;
//                             i3 = i3_final;
//                             break;
//                         }
//                         case ERT::next2 : {
//                             i3 = i3_final;
//                             break;
//                         }
//                         default : {  // exhaustive, with ERT::next3
//                             break;
//                         }
//                     }
//                 }
//
//     return buffered_extrap_pot.extract_energy();
// }
//
// template <std::floating_point FP, std::size_t NDIM>
// auto total_quadruplet_potential_energy_periodic(
//
// ) -> FP
// {}

}  // namespace estim
