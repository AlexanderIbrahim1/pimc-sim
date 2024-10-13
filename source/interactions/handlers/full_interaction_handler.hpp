#pragma once

#include <concepts>
#include <cstddef>
#include <span>
#include <utility>

#include <interactions/handlers/interaction_handler_concepts.hpp>
#include <interactions/three_body/potential_concepts.hpp>
#include <interactions/two_body/potential_concepts.hpp>
#include <worldline/worldline.hpp>

namespace interact
{

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires PairPointPotential<PointPotential, FP, NDIM>
class FullPairInteractionHandler
{
public:
    explicit FullPairInteractionHandler(PointPotential pot)
        : pot_ {std::move(pot)}
    {}

    /*
        Calculate the total pair interaction energy between the particle at index `i_particle` and all
        other particles, taking into account that we should avoid the self-interaction.
    */
    constexpr auto operator()(std::size_t i_timeslice, std::size_t i_particle, const worldline::Worldlines<FP, NDIM>& worldlines) const noexcept -> FP
    {
        const auto timeslice = worldlines.timeslice(i_timeslice);
        const auto& point = timeslice[i_particle];

        auto pot_energy = FP {};
        for (std::size_t i {0}; i < timeslice.size(); ++i) {
            if (i == i_particle) {
                continue;
            }

            pot_energy += pot_(particle, timeslice[i]);
        }

        return pot_energy;
    }

    constexpr auto point_potential() const -> const PointPotential&
    {
        return pot_;
    }

private:
    PointPotential pot_;
};

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires TripletPointPotential<PointPotential, FP, NDIM>
class FullTripletInteractionHandler
{
public:
    explicit FullTripletInteractionHandler(PointPotential pot)
        : pot_ {std::move(pot)}
    {}

    /*
        Calculate the total triplet interaction energy between the particle at index `i_particle` and all
        other particles, taking into account that we should avoid the self-interaction.
    */
    constexpr auto operator()(std::size_t i_timeslice, std::size_t i_particle, const worldline::Worldlines<FP, NDIM>& worldlines) const noexcept -> FP
    {
        const auto timeslice = worldlines.timeslice(i_timeslice);
        const auto& particle = timeslice[i_particle];

        auto pot_energy = FP {};
        for (std::size_t i0 {0}; i0 < timeslice.size() - 1; ++i0) {
            if (i0 == i_particle) {
                continue;
            }
            for (std::size_t i1 {i0 + 1}; i1 < timeslice.size(); ++i1) {
                if (i1 == i_particle) {
                    continue;
                }
                pot_energy += pot_(particle, timeslice[i0], timeslice[i1]);
            }
        }

        return pot_energy;
    }

    constexpr auto point_potential() const -> const PointPotential&
    {
        return pot_;
    }

private:
    PointPotential pot_;
};

}  // namespace interact
