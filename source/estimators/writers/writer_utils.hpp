#pragma once

#include <string_view>

namespace estim
{

namespace writer_utils
{
constexpr auto DEFAULT_KINETIC_OUTPUT_FILENAME = std::string_view {"kinetic.dat"};
constexpr auto DEFAULT_PAIR_POTENTIAL_OUTPUT_FILENAME = std::string_view {"pair_potential.dat"};
constexpr auto DEFAULT_RMS_CENTROID_DISTANCE_OUTPUT_FILENAME = std::string_view {"rms_centroid_distance.dat"};
constexpr auto DEFAULT_ABSOLUTE_CENTROID_DISTANCE_OUTPUT_FILENAME = std::string_view {"absolute_centroid_distance.dat"};
}  // namespace writer_utils

}  // namespace estim
