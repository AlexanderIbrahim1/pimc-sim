#pragma once

#include <concepts>

namespace envir_utils {

/*
This is the constant which, when divided by the mass of a particle (in units of AMU), gives
the thermodynamic lambda of that particle (in units of Angstrom^2 * Kelvin).

This result is pre-calculated to minimize floating-point rounding differences for different
floating-point types.

This is equivalent to:
```
    const auto hbar = constants::HBAR_IN_JOULES_SECONDS<FP>;
    const auto boltz = constants::BOLTZMANN_CONSTANT_IN_JOULES_PER_KELVIN<FP>;
    const auto kg_per_amu = conversions::KILOGRAMS_PER_AMU<FP>;
    const auto ang_per_m = conversions::ANGSTROMS_PER_METRE<FP>;

    const auto coefficient = hbar * hbar * ang_per_m * ang_per_m / kg_per_amu / boltz;

    return 0.5 * coefficient
```
*/
template <std::floating_point FP>
static constexpr FP LAMBDA_CONVERSION_FACTOR;

template <>
constexpr double LAMBDA_CONVERSION_FACTOR<double> = 24.254350505951773;

template <>
constexpr float LAMBDA_CONVERSION_FACTOR<float> = 24.254350505951773f;

}  // namespace envir_utils
