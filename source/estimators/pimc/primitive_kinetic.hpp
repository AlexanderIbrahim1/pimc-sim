#pragma once

#include <concepts>

#include <constants/conversions.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <worldline/worldline.hpp>

// TODO: account for periodicity when calculating the distance?
// - could happen in cases where the beads are *EXTREMELY* spread out?
// - very unlikely in practice, but I guess it could happen in theory???
//   - and accounting for it wouldn't hurt, but would be annoying to implement

namespace estim
{

// TODO: simplify this, since we also have functions that get these quantities with units of wavenumbers
// instead of Kelvin
template <std::floating_point FP>
constexpr auto primitive_kinetic_energy(
    const envir::Environment<FP>& environment,
    FP total_dist_squared,
    std::size_t ndim
) noexcept -> FP
{
    const auto n_particles = environment.n_particles();
    const auto lambda = environment.thermodynamic_lambda_kelvin();
    const auto tau = environment.thermodynamic_tau_kelvin();
    const auto beta = environment.thermodynamic_beta_kelvin();

    const auto thermal_kinetic_energy = 0.5 * static_cast<FP>(ndim * n_particles) / tau;
    const auto vibration_correction_energy = total_dist_squared / (FP {4.0} * tau * beta * lambda);

    const auto energy_in_kelvin = thermal_kinetic_energy - vibration_correction_energy;
    const auto energy_in_wvn = energy_in_kelvin * conversions::WAVENUMBERS_PER_KELVIN<FP>;

    return energy_in_wvn;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto total_primitive_kinetic_energy(
    const worldline::Worldlines<FP, NDIM>& worldlines,
    const envir::Environment<FP>& environment
) noexcept -> FP
{
    auto total_dist_squared = FP {};

    const auto n_particles = worldlines.n_worldlines();
    const auto n_timeslices = worldlines.n_timeslices();

    for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
        // take care of the boundary case, where the 0th timeslice touches the last one
        total_dist_squared +=
            coord::distance_squared(worldlines.get(0, i_part), worldlines.get(n_timeslices - 1, i_part));

        // take care of the uninterrupted chain separately
        for (std::size_t i_tslice {0}; i_tslice < n_timeslices - 1; ++i_tslice) {
            total_dist_squared +=
                coord::distance_squared(worldlines.get(i_tslice, i_part), worldlines.get(i_tslice + 1, i_part));
        }
    }

    return primitive_kinetic_energy(environment, total_dist_squared, NDIM);
}

}  // namespace estim
