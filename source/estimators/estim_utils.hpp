#pragma once

#include <concepts>

namespace estim_utils
{

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

constexpr auto DEFAULT_BLOCK_INDEX_PADDING = int {5};
constexpr auto DEFAULT_SINGLE_VALUE_PRECISION = int {8};

}  // namespace estim_utils
