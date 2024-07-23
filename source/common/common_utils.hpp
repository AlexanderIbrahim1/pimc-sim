#pragma once

#include <cmath>
#include <concepts>
#include <iterator>
#include <tuple>

namespace common_utils
{

template <typename T>
concept Numeric = std::floating_point<T> || std::integral<T>;

template <typename T>
struct always_false
{
    static constexpr bool value = false;
};

template <std::floating_point FP>
constexpr auto smooth_01_transition(FP x, FP x_min, FP x_max) -> FP
{
    if (x <= x_min) {
        return FP {0.0};
    }
    else if (x >= x_max) {
        return FP {1.0};
    }
    else {
        FP k = (x - x_min) / (x_max - x_min);
        FP inner = FP {1.0} - std::cos(PI_CONST<FP> * k);
        return FP {0.5} * inner;
    }
}

template <typename NumericType1, typename NumericType2>
constexpr auto is_same_sign(NumericType1 a, NumericType2 b) -> bool
{
    return ((a < NumericType1 {0.0}) == (b < NumericType2 {0.0}));
}

template <typename NumericType>
constexpr auto sign(NumericType x)
{
    if (x >= NumericType {0.0}) {
        return NumericType {1.0};
    }
    else {
        return NumericType {-1.0};
    }
}

}  // namespace common_utils
