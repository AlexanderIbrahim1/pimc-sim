#pragma once

#include <concepts>
#include <cstddef>
#include <vector>

#include <environment/environment.hpp>
#include <interactions/two_body/potential_concepts.hpp>
#include <worldline/worldline.hpp>

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
    if (worldlines.empty()) {
        return FP {0.0};
    }

    auto total_pair_potential_energy = FP {0.0};

    for (const auto& wline : worldlines) {
        const auto& points = wline.points();

        for (std::size_t ip0 {0}; ip0 < points.size() - 1; ++ip0) {
            const auto p0 = points[ip0];
            for (std::size_t ip1 {ip0 + 1}; ip1 < points.size(); ++ip1) {
                total_pair_potential_energy += potential(p0, points[ip1]);
            }
        }
    }

    total_pair_potential_energy /= static_cast<FP>(environment.n_timeslices());

    return total_pair_potential_energy;
}

}  // namespace estim
