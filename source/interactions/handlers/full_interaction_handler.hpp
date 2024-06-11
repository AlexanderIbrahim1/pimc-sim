#pragma once

#include <concepts>
#include <cstddef>
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
    using Worldline = worldline::Worldline<FP, NDIM>;

public:
    explicit FullPairInteractionHandler(PointPotential pot)
        : pot_ {std::move(pot)}
    {}

    constexpr auto operator()(std::size_t i_particle, const Worldline& worldline) const noexcept -> FP
    {
        auto pot_energy = FP {};

        const auto& points = worldline.points();
        for (std::size_t i {0}; i < worldline.size(); ++i) {
            if (i == i_particle) {
                continue;
            }

            pot_energy += pot_(points[i_particle], points[i]);
        }

        return pot_energy;
    }

    constexpr auto point_potential() const -> const PointPotential& {
        return pot_;
    }

private:
    PointPotential pot_;
};

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires TripletPointPotential<PointPotential, FP, NDIM>
class FullTripletInteractionHandler
{
    using Worldline = worldline::Worldline<FP, NDIM>;

public:
    explicit FullTripletInteractionHandler(PointPotential pot)
        : pot_ {std::move(pot)}
    {}

    constexpr auto operator()(std::size_t i_particle, const Worldline& worldline) const noexcept -> FP
    {
        auto pot_energy = FP {};

        const auto& points = worldline.points();

        for (std::size_t i0 {0}; i0 < worldline.size() - 1; ++i0) {
            if (i0 == i_particle) {
                continue;
            }
            for (std::size_t i1 {i0 + 1}; i1 < worldline.size(); ++i1) {
                if (i1 == i_particle) {
                    continue;
                }
                pot_energy += pot_(points[i_particle], points[i0], points[i1]);
            }
        }

        return pot_energy;
    }

    constexpr auto point_potential() const -> const PointPotential& {
        return pot_;
    }

private:
    PointPotential pot_;
};

}  // namespace interact
