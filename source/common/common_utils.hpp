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

constexpr auto DEFAULT_WRITER_BLOCK_INDEX_PADDING = int {5};
constexpr auto DEFAULT_WRITER_SINGLE_VALUE_PRECISION = int {8};

}  // namespace common_utils
