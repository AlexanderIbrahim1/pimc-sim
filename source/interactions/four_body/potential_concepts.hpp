#pragma once

#include <concepts>

#include <coordinates/attard/four_body.hpp>
#include <coordinates/cartesian.hpp>

namespace interact
{

template <typename Potential, typename FP>
concept BufferedQuadrupletPotential = requires(Potential pot) {
    requires std::is_floating_point_v<FP>;
    {
        pot.add_sample(coord::FourBodySideLengths<FP> {})
    } -> std::same_as<void>;

    {
        pot.extract_energy()
    } -> std::same_as<FP>;
};

template <typename Potential, typename FP, std::size_t NDIM>
concept BufferedQuadrupletPointPotential = requires(Potential pot) {
    requires std::is_floating_point_v<FP>;
    {
        pot.add_sample(
            coord::Cartesian<FP, NDIM> {},
            coord::Cartesian<FP, NDIM> {},
            coord::Cartesian<FP, NDIM> {},
            coord::Cartesian<FP, NDIM> {}
        )
    } -> std::same_as<void>;

    {
        pot.extract_energy()
    } -> std::same_as<FP>;
};

}  // namespace interact
