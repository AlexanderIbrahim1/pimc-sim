#pragma once

#include <concepts>

namespace mymath
{

template <typename T>
concept Number = std::integral<T> || std::floating_point<T>;

template <Number T>
constexpr auto add(T a, T b) -> T {
    return a + b;
}

}  // namespace mymath
