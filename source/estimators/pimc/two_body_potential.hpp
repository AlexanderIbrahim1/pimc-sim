#pragma once

#include <concepts>
#include <cstddef>
#include <vector>

#include <coordinates/box_sides.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <interactions/two_body/potential_concepts.hpp>
#include <worldline/worldline.hpp>

namespace impl_estim
{

template <typename PointPotential, std::floating_point FP, std::size_t NDIM, bool IsPeriodic>
constexpr auto total_pair_potential_energy_maybe_periodic(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const PointPotential& potential,
    const envir::Environment<FP>& environment
) noexcept -> FP
{
    if constexpr (IsPeriodic) {
        static_assert(interact::PeriodicPairPointPotential<PointPotential, FP, NDIM>);
    } else {
        static_assert(interact::PairPointPotential<PointPotential, FP, NDIM>);
    }

    if (worldlines.empty()) {
        return FP {0.0};
    }

    auto total_pair_potential_energy = FP {0.0};

    for (const auto& wline : worldlines) {
        const auto& points = wline.points();

        for (std::size_t ip0 {0}; ip0 < points.size() - 1; ++ip0) {
            const auto p0 = points[ip0];
            for (std::size_t ip1 {ip0 + 1}; ip1 < points.size(); ++ip1) {
                if constexpr (IsPeriodic) {
                    total_pair_potential_energy += potential.within_box_cutoff(p0, points[ip1]);
                } else {
                    total_pair_potential_energy += potential(p0, points[ip1]);
                }
            }
        }
    }

    total_pair_potential_energy /= static_cast<FP>(environment.n_timeslices());

    return total_pair_potential_energy;
}

}


namespace estim
{

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires interact::PairPointPotential<PointPotential, FP, NDIM>
constexpr auto total_pair_potential_energy(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const PointPotential& potential,
    const envir::Environment<FP>& environment
) noexcept -> FP
{
    return impl_estim::total_pair_potential_energy_maybe_periodic<PointPotential, FP, NDIM, false>(worldlines, potential, environment);
}

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires interact::PeriodicPairPointPotential<PointPotential, FP, NDIM>
constexpr auto total_pair_potential_energy_periodic(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const PointPotential& potential,
    const envir::Environment<FP>& environment
) noexcept -> FP
{
    return impl_estim::total_pair_potential_energy_maybe_periodic<PointPotential, FP, NDIM, true>(worldlines, potential, environment);
}

}  // namespace estim
