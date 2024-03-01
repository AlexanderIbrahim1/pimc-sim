#pragma once

#include <concepts>

#include <coordinates/cartesian.hpp>

namespace interact
{

template <typename Potential>
concept PairPotential = requires(Potential pot) {
    {
        pot(0.0)
    } -> std::floating_point;
};

template <typename Potential, typename FP, std::size_t NDIM>
concept PairPointPotential = requires(Potential pot) {
    requires std::is_floating_point_v<FP>;
    {
        pot(coord::Cartesian<FP, NDIM> {}, coord::Cartesian<FP, NDIM> {})
    } -> std::same_as<FP>;
};

template <typename Potential, typename FP, std::size_t NDIM>
concept PeriodicPairPointPotential = requires(Potential pot) {
    requires std::is_floating_point_v<FP>;
    {
        pot(coord::Cartesian<FP, NDIM> {}, coord::Cartesian<FP, NDIM> {})
    } -> std::same_as<FP>;
    {
        pot.within_box_cutoff(coord::Cartesian<FP, NDIM> {}, coord::Cartesian<FP, NDIM> {})
    } -> std::same_as<FP>;
};

}  // namespace interact
