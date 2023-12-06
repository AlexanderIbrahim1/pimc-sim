#pragma once

#include <concepts>

namespace coord
{

template <std::floating_point FP>
static constexpr FP EPSILON_CARTESIAN_ZERO_DIVIDE;

template <>
constexpr float EPSILON_CARTESIAN_ZERO_DIVIDE<float> = 1.0e-4f;

template <>
constexpr double EPSILON_CARTESIAN_ZERO_DIVIDE<double> = 1.0e-8;

template <std::floating_point FP>
static constexpr FP EPSILON_BOX_SEPARATION;

template <>
constexpr float EPSILON_BOX_SEPARATION<float> = 1.0e-4f;

template <>
constexpr double EPSILON_BOX_SEPARATION<double> = 1.0e-8;

static constexpr int CARTESIAN_OSTREAM_PRECISION = 6;

}  // namespace coord