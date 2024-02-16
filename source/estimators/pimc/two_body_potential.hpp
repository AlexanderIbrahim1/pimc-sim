#pragma once

#include <concepts>
#include <cstddef>
#include <vector>

#include <environment/environment.hpp>
#include <interactions/handlers/periodic_full_pair_interaction_handler.hpp>
#include <worldline/worldline.hpp>

namespace estim
{

template <std::floating_point FP, std::size_t NDIM>
constexpr auto total_pair_potential_energy(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const interact::PairDistancePotential auto& potential,
    const envir::Environment<FP>& environment
) noexcept -> FP
{
    if (worldlines.empty()) {
        return FP {0.0};
    }

    auto total_pair_potential = FP {0.0};

    for (const auto& wline : worldlines) {
        const auto& points = wline.points();

        for (std::size_it ip0 {0}; ip0 < points.size() - 1; ++ip0) {
            for (std::size_it ip1 {ip0 + 1}; ip1 < points.size(); ++ip1) {
            }
        }
    }
}

}  // namespace estim
