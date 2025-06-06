#pragma once

#include <cmath>
#include <concepts>

#include <constants/mass.hpp>

namespace constants
{

template <std::floating_point FP>
static constexpr FP HBAR_IN_JOULES_SECONDS;

template <>
constexpr double HBAR_IN_JOULES_SECONDS<double> = 1.054'571'817e-34;

template <>
constexpr float HBAR_IN_JOULES_SECONDS<float> = 1.054'571'817e-34f;

template <std::floating_point FP>
static constexpr FP BOLTZMANN_CONSTANT_IN_JOULES_PER_KELVIN;

template <>
constexpr double BOLTZMANN_CONSTANT_IN_JOULES_PER_KELVIN<double> = 1.380'649e-23;

template <>
constexpr float BOLTZMANN_CONSTANT_IN_JOULES_PER_KELVIN<float> = 1.380'649e-23f;

/* Units of [wavenumber] * [Angstrom]^[9] */
template <std::floating_point FP>
constexpr FP C9_ATM_COEFFICIENT_HINDE2008 = static_cast<FP>(34336.2);

}  // namespace constants
