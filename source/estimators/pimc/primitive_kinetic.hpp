#pragma once

#include <concepts>
#include <vector>

#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <worldline/worldline.hpp>

// TODO: account for periodicity when calculating the distance?
// - could happen in cases where the beads are *EXTREMELY* spread out?
// - very unlikely in practice, but I guess it could happen in theory???
//   - and accounting for it wouldn't hurt, but would be annoying to implement

namespace estim
{

template <std::floating_point FP>
constexpr auto primitive_kinetic_energy(
    const envir::Environment<FP>& environment,
    FP total_dist_squared,
    std::size_t ndim
) noexcept -> FP
{
    const auto n_particles = environment.n_particles();
    const auto lambda = environment.thermodynamic_lambda();
    const auto tau = environment.thermodynamic_tau();
    const auto beta = environment.thermodynamic_beta();

    const auto thermal_kinetic_energy = 0.5 * static_cast<FP>(ndim * n_particles) / tau;
    const auto vibration_correction_energy = total_dist_squared / (FP {4.0} * tau * beta * lambda);

    return thermal_kinetic_energy - vibration_correction_energy;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto total_primitive_kinetic_energy(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const envir::Environment<FP>& environment
) noexcept -> FP
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

        // take care of the boundary case, where the 0th timeslice touches the last one
        total_dist_squared += coord::distance_squared(points[0], points.back());

        // take care of the uninterrupted chain separately
        for (std::size_t i_part {0}; i_part < points.size() - 1; ++i_part) {
            total_dist_squared += coord::distance_squared(points[i_part], points[i_part + 1]);
        }
    }

    return primitive_kinetic_energy(environment, total_dist_squared, NDIM);
}

}  // namespace estim
