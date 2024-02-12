#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
#include <worldline/worldline.hpp>

namespace interact
{

template <typename T>
concept InteractionHandler = requires(T t) {
    {
        t(0, {})
    } -> std::floating_point;
};

template <std::floating_point FP, std::size_t NDIM, interact::PairDistancePotential Potential>
class PeriodicFullPairInteractionHandler
{
public:
    explicit PeriodicFullPairInteractionHandler(Potential pot, coord::BoxSides<FP, NDIM> box)
        : pot_ {std::move(pot)}
        , box_ {std::move(box)}
    {}

    constexpr auto operator()(std::size_t i_particle, const worldline::Worldline<FP, NDIM>& worldline) const noexcept
        -> FP
    {
        auto pot_energy = FP {};

        const auto& points = worldline.points();
        for (std::size_t i {0}; i < worldline.size(); ++i) {
            if (i == i_particle) {
                continue;
            }

            const auto distance = coord::distance_periodic(points[i_particle], points[i], box_);
            pot_energy += pot_(distance);
        }

        return pot_energy;
    }

private:
    Potential pot_;
    coord::BoxSides<FP, NDIM> box_;
};

}  // namespace interact
