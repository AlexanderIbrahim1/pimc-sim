#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <interactions/two_body/potential_concepts.hpp>
#include <worldline/worldline.hpp>

namespace interact
{

template <typename T>
concept InteractionHandler = requires(T t) {
    {
        t(0, {})
    } -> std::floating_point;
};

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

private:
    PointPotential pot_;
};

}  // namespace interact
