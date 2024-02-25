#pragma once

#include <concepts>
#include <iterator>

namespace common_utils
{

template <typename T>
concept Numeric = std::floating_point<T> || std::integral<T>;

template <typename T>
struct always_false
{
    static constexpr bool value = false;
};

}  // namespace common_utils
