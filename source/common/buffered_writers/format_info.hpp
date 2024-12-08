#pragma once

#include <array>
#include <cstddef>


namespace common::writers
{

template <std::size_t N>
struct FormatInfo
{
    int block_index_padding;
    std::size_t spacing;
    std::array<int, N> integer_padding;
    std::array<int, N> floating_point_precision;
};

}  // namespace common::writers
