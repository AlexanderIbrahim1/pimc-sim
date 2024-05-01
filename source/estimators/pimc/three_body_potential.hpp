#pragma once

#include <concepts>
#include <cstddef>
#include <vector>

#include <coordinates/box_sides.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <interactions/three_body/potential_concepts.hpp>
#include <worldline/worldline.hpp>

namespace estim
{

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires interact::TripletPointPotential<PointPotential, FP, NDIM>
constexpr auto total_triplet_potential_energy(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const PointPotential& potential,
    const envir::Environment<FP>& environment
) noexcept -> FP
{
    if (worldlines.empty()) {
        return FP {0.0};
    }

    auto total_triplet_pot_energy = FP {0.0};

    for (const auto& wline : worldlines) {
        const auto& points = wline.points();

        for (std::size_t ip0 {0}; ip0 < points.size() - 2; ++ip0) {
            const auto p0 = points[ip0];
            for (std::size_t ip1 {ip0 + 1}; ip1 < points.size() - 1; ++ip1) {
                const auto p1 = points[ip1];
                for (std::size_t ip2 {ip1 + 1}; ip2 < points.size(); ++ip2) {
                    const auto p2 = points[ip2];
                    total_triplet_pot_energy += potential(p0, p1, p2);
                }
            }
        }
    }

    total_triplet_pot_energy /= static_cast<FP>(environment.n_timeslices());

    return total_triplet_pot_energy;
}

// a lot of code duplication for a single line, but I tried the if-constexpr approach and it was a bit worse IMO
// - the concepts overlap, etc.
// - there's a lot of opportunity for confusion
// - maybe I'll handle this better in a later change
template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires interact::PeriodicTripletPointPotential<PointPotential, FP, NDIM>
constexpr auto total_triplet_potential_energy_periodic(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const PointPotential& potential,
    const envir::Environment<FP>& environment
) noexcept -> FP
{
    if (worldlines.empty()) {
        return FP {0.0};
    }

    auto total_triplet_pot_energy = FP {0.0};

    for (const auto& wline : worldlines) {
        const auto& points = wline.points();

        for (std::size_t ip0 {0}; ip0 < points.size() - 2; ++ip0) {
            const auto p0 = points[ip0];
            for (std::size_t ip1 {ip0 + 1}; ip1 < points.size() - 1; ++ip1) {
                const auto p1 = points[ip1];
                for (std::size_t ip2 {ip1 + 1}; ip2 < points.size(); ++ip2) {
                    const auto p2 = points[ip2];
                    total_triplet_pot_energy += potential.within_box_cutoff(p0, p1, p2);
                }
            }
        }
    }

    total_triplet_pot_energy /= static_cast<FP>(environment.n_timeslices());

    return total_triplet_pot_energy;
}

}  // namespace estim
