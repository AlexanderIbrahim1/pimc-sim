#pragma once

#include <concepts>

namespace geom
{

template <std::floating_point FP>
static constexpr FP EPSILON_MINIMUM_LATTICE_VECTOR_NORM_SQUARED;

template <>
constexpr float EPSILON_MINIMUM_LATTICE_VECTOR_NORM_SQUARED<float> = 1.0e-4f;

template <>
constexpr double EPSILON_MINIMUM_LATTICE_VECTOR_NORM_SQUARED<double> = 1.0e-8f;

}  // namespace geom
