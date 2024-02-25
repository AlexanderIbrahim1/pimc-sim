#pragma once

#include <concepts>
#include <string_view>

namespace estim_utils
{

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

constexpr auto DEFAULT_BLOCK_INDEX_PADDING = int {5};
constexpr auto DEFAULT_SINGLE_VALUE_PRECISION = int {8};

constexpr auto DEFAULT_KINETIC_OUTPUT_FILENAME = std::string_view {"kinetic.dat"};
constexpr auto DEFAULT_PAIR_POTENTIAL_OUTPUT_FILENAME = std::string_view {"pair_potential.dat"};
constexpr auto DEFAULT_CENTROID_OUTPUT_FILENAME = std::string_view {"centroid.dat"};

}  // namespace estim_utils
