#pragma once

#include <concepts>
#include <cstddef>

namespace pimc
{

template <std::floating_point FP>
struct BisectionLevelMoveInfo
{
    FP upper_level_frac;
    std::size_t lower_level;
};

}  // namespace pimc
