#pragma once

#include <concepts>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace mathtools_utils
{

[[maybe_unused]] static void check_in_bounds(std::size_t index, std::size_t size)
{
    if (index >= size) {
        auto err_msg = std::stringstream {};
        err_msg << "Out of bounds access.\n";
        err_msg << "size = " << size << '\n';
        err_msg << "index = " << index << '\n';
        throw std::runtime_error(err_msg.str());
    }
}

[[maybe_unused]] static void ctr_check_positive(std::size_t value, std::string_view name)
{
    if (value < 1) {
        auto err_msg = std::stringstream {};
        err_msg << "'" << name << "' must be positive.\n";
        err_msg << "Found: " << name << " = " << value << '\n';
        throw std::runtime_error(err_msg.str());
    }
}

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

[[maybe_unused]] static void ctr_check_data_size_at_least_two(std::size_t size)
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

}  // namespace mathtools_utils
