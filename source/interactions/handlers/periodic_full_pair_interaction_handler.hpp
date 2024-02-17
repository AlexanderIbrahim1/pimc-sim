#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
#include <interactions/two_body/two_body_pointwise_wrapper.hpp>
#include <worldline/worldline.hpp>

namespace interact
{

template <typename T>
concept InteractionHandler = requires(T t) {
    {
        t(0, {})
    } -> std::floating_point;
};

template <typename Potential, std::floating_point FP, std::size_t NDIM>
class FullPairInteractionHandler
{
    static_assert(interact::PairPointPotential<Potential, FP, NDIM>);
    using Worldline = worldline::Worldline<FP, NDIM>;

public:
    explicit FullPairInteractionHandler(Potential pot)
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
    Potential pot_;
};

}  // namespace interact
