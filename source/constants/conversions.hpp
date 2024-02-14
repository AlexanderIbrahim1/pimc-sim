#pragma once

#include <concepts>

namespace conversions
{

template <std::floating_point FP>
static constexpr FP KILOGRAMS_PER_AMU;

template <>
constexpr double KILOGRAMS_PER_AMU<double> = 1.660'540'200e-27;

template <>
constexpr float KILOGRAMS_PER_AMU<float> = 1.660'540'200e-27f;

template <std::floating_point FP>
static constexpr FP ANGSTROMS_PER_METRE;

template <>
constexpr double ANGSTROMS_PER_METRE<double> = 1.0e10;

template <>
constexpr float ANGSTROMS_PER_METRE<float> = 1.0e10f;

}  // namespace conversions
