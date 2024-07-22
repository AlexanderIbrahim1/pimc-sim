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

template<std::floating_point FP>
static constexpr FP PI_CONST;

template <>
constexpr double PI_VALUE<double> = static_cast<double>(M_PI);

template <>
constexpr float PI_VALUE<float> = static_cast<float>(M_PI);


}  // namespace constants
