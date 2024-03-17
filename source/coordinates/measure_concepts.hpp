#pragma once

#include <concepts>
#include <cstddef>

#include <coordinates/cartesian.hpp>

namespace coord
{

template <typename T, typename FP, std::size_t NDIM>
concept DistanceCalculator = requires(T t) {
    requires std::is_floating_point_v<FP>;

    {
        t(Cartesian<FP, NDIM> {}, Cartesian<FP, NDIM> {})
    } -> std::floating_point;
};

template <typename T, typename FP, std::size_t NDIM>
concept DistanceSquaredCalculator = requires(T t) {
    requires std::is_floating_point_v<FP>;

    {
        t(Cartesian<FP, NDIM> {}, Cartesian<FP, NDIM> {})
    } -> std::floating_point;
};

}  // namespace coord
