#pragma once

#include <concepts>

#include <coordinates/cartesian.hpp>

namespace coord
{

template <std::floating_point FP, std::size_t NDIM>
constexpr auto dot_product(const Cartesian<FP, NDIM>& point0, const Cartesian<FP, NDIM>& point1) noexcept -> FP
{
    auto result = FP {};

    for (std::size_t i_dim {}; i_dim < NDIM; ++i_dim) {
        const auto term = point0[i_dim] * point1[i_dim];
        result += term;
    }

    return result;
}

}  // namespace coord
