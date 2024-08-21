#pragma once

#include <concepts>

#include <coordinates/attard.hpp>
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

}  // namespace interact
