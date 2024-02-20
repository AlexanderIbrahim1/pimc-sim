#pragma once

#include <concepts>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace interp
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
        slopes[i] = (ydata[i + 1] - ydata[i]) / dx;
    }

    return slopes;
}

template <std::floating_point FP>
class RegularLinearInterpolator
{
public:
    RegularLinearInterpolator(std::vector<FP> ydata, FP xmin, FP xmax)
        : ydata_ {std::move(ydata)}
        , xmin_ {xmin}
        , xmax_ {xmax}
    {
        ctr_check_data_size_at_least_two(ydata_.size());
        ctr_check_min_max_order(xmin, xmax);

        dx_ = (xmax_ - xmin_) / static_cast<FP>(ydata_.size() - 1);

        slopes_ = ctr_create_slopes(ydata_, dx_);
    }

    constexpr auto operator()(FP x) const noexcept -> FP
    {
        const auto i_lower = static_cast<std::size_t>((x - xmin_) / dx_);
        const auto x_lower = xmin_ + static_cast<FP>(i_lower) * dx_;

        const auto slope = slopes_[i_lower];
        const auto x_step = x - x_lower;
        const auto y_intercept = ydata_[i_lower];

        return slope * x_step + y_intercept;
    }

    constexpr auto at(FP x) const -> FP
    {
        if (x < xmin_ || x >= xmax_) {
            auto err_msg = std::stringstream {};
            err_msg << "Received an out-of-bounds intercept access.\n";
            err_msg << "All accesses must be between " << xmin_ << " and " << xmax_ << '\n';
            err_msg << "Found: x = " << x << '\n';
            throw std::runtime_error(err_msg.str());
        }

        return this->operator()(x);
    }

private:
    std::vector<FP> ydata_;
    FP xmin_;
    FP xmax_;
    FP dx_;
    std::vector<FP> slopes_;
};

}  // namespace interp
