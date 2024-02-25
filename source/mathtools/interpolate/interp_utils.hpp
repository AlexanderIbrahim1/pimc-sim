#pragma once

#include <concepts>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace mathtools_utils
{

template <std::floating_point FP>
constexpr void ctr_check_min_max_order(FP xmin, FP xmax)
{
    if (xmin >= xmax) {
        auto err_msg = std::stringstream {};
        err_msg << "Interpolation requires that 'xmin < xmax'.\n";
        err_msg << "Found: xmin = " << xmin << ", xmax = " << xmax << '\n';
        throw std::runtime_error {err_msg.str()};
    }
}

static void ctr_check_data_size_at_least_two(std::size_t size)
{
    if (size < 2) {
        auto err_msg = std::stringstream {};
        err_msg << "At least two elements are required for interpolation.\n";
        err_msg << "Found: size = " << size << '\n';
        throw std::runtime_error {err_msg.str()};
    }
}

template <std::floating_point FP>
constexpr auto ctr_create_slopes(const std::vector<FP>& ydata, FP dx) noexcept -> std::vector<FP>
{
    // depends on invariants already established in the constructor, to work properly
    const auto size = ydata.size() - 1;

    auto slopes = std::vector<FP> {};
    slopes.reserve(size);

    for (std::size_t i {0}; i < size; ++i) {
        const auto slope = (ydata[i + 1] - ydata[i]) / dx;
        slopes.push_back(slope);
    }

    return slopes;
}

}  // namespace interp_utils
