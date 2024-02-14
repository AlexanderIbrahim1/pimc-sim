#pragma once

#include <concepts>

namespace constants
{

template <std::floating_point FP>
static constexpr FP H2_MASS_IN_AMU = static_cast<FP>(2.015650642);

}
