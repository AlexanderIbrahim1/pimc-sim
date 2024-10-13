#pragma once

#include <concepts>
#include <cstddef>
#include <vector>

#include <interactions/two_body/potential_concepts.hpp>
#include <worldline/worldline.hpp>

namespace impl_estim
{

template <typename PointPotential, std::floating_point FP, std::size_t NDIM, bool IsPeriodic>
constexpr auto total_pair_potential_energy_maybe_periodic(
    const worldline::Worldlines<FP, NDIM>& worldlines,
    const PointPotential& potential
) noexcept -> FP
{
    if constexpr (IsPeriodic) {
        static_assert(interact::PeriodicPairPointPotential<PointPotential, FP, NDIM>);
    }
    else {
        static_assert(interact::PairPointPotential<PointPotential, FP, NDIM>);
    }

    const auto n_timeslices = worldlines.n_timeslices();
    const auto n_particles = worldlines.n_worldlines();

    auto total_pair_potential_energy = FP {0.0};

    for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
        const auto timeslice = worldlines.timeslice(i_tslice);

        for (std::size_t ip0 {0}; ip0 < n_particles - 1; ++ip0) {
            const auto p0 = timeslice[ip0];
            for (std::size_t ip1 {ip0 + 1}; ip1 < n_particles; ++ip1) {
                const auto p1 = timeslice[ip1];
                if constexpr (IsPeriodic) {
                    total_pair_potential_energy += potential.within_box_cutoff(p0, p1);
                }
                else {
                    total_pair_potential_energy += potential(p0, p1);
                }
            }
        }
    }

    total_pair_potential_energy /= static_cast<FP>(n_timeslices);

    return total_pair_potential_energy;
}

}  // namespace impl_estim

namespace estim
{

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires interact::PairPointPotential<PointPotential, FP, NDIM>
constexpr auto total_pair_potential_energy(
    const worldline::Worldlines<FP, NDIM>& worldlines,
    const PointPotential& potential
) noexcept -> FP
{
    return impl_estim::total_pair_potential_energy_maybe_periodic<PointPotential, FP, NDIM, false>(
        worldlines, potential
    );
}

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires interact::PeriodicPairPointPotential<PointPotential, FP, NDIM>
constexpr auto total_pair_potential_energy_periodic(
    const worldline::Worldlines<FP, NDIM>& worldlines,
    const PointPotential& potential
) noexcept -> FP
{
    return impl_estim::total_pair_potential_energy_maybe_periodic<PointPotential, FP, NDIM, true>(
        worldlines, potential
    );
}

}  // namespace estim
