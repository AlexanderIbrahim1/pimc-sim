#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace mathtools
{

enum class OutOfRangePolicy
{
    DO_NOTHING,
    THROW
};

template <std::floating_point FP>
class Histogram
{
public:
    Histogram(FP min, FP max, std::size_t n_bins, OutOfRangePolicy policy = OutOfRangePolicy::DO_NOTHING)
        : bins_(n_bins, 0)
        , min_ {min}
        , max_ {max}
        , step_size_ {calculate_step_size_(min, max, n_bins)}
        , policy_ {policy}
    {}

    Histogram(FP min, FP max, std::vector<std::uint64_t> bins, OutOfRangePolicy policy = OutOfRangePolicy::DO_NOTHING)
        : bins_ {std::move(bins)}
        , min_ {min}
        , max_ {max}
        , step_size_ {calculate_step_size_(min, max, bins_.size())}
        , policy_ {policy}
    {}

    constexpr void reset() noexcept
    {
        std::fill(std::begin(bins_), std::end(bins_), std::uint64_t {0});
    }

    constexpr auto bins() const noexcept -> const std::vector<std::uint64_t>&
    {
        return bins_;
    }

    /* NOTE: I need all these getters because saving the histogram state requires this information */
    constexpr auto min() const noexcept -> FP
    {
        return min_;
    }

    constexpr auto max() const noexcept -> FP
    {
        return max_;
    }

    constexpr auto policy() const noexcept -> OutOfRangePolicy
    {
        return policy_;
    }

    constexpr auto add(FP value, std::uint64_t count = 1) -> bool
    {
        if (policy_ == OutOfRangePolicy::THROW) {
            auto err_msg = std::stringstream {};
            err_msg << "Received an entry outside of the bounds of the histogram!\n";
            err_msg << std::scientific << std::setprecision(8);
            err_msg << "Found: " << value << '\n';
            err_msg << "min bound = " << min_ << '\n';
            err_msg << "max bound = " << max_ << '\n';

            throw std::runtime_error {err_msg.str()};
        }

        if (value < min_ || value >= max_) {
            return false;
        }

        const auto fp_index = std::floor((value - min_) / step_size_);
        const auto index = static_cast<std::uint64_t>(fp_index);

        bins_[index] += count;

        return true;
    }

    constexpr void set_policy(OutOfRangePolicy policy) noexcept
    {
        policy_ = policy;
    }

private:
    std::vector<std::uint64_t> bins_;
    FP min_;
    FP max_;
    FP step_size_;
    OutOfRangePolicy policy_;

    constexpr auto calculate_step_size_(FP min, FP max, std::size_t n_bins) const -> FP
    {
        if (n_bins < 1) {
            throw std::runtime_error {"The histogram cannot be built with 0 bins\n"};
        }

        if (min >= max) {
            auto err_msg = std::stringstream {};
            err_msg << "The minimum of the histogram must be less than the maximum.\n";
            err_msg << std::scientific << std::setprecision(8);
            err_msg << "Found: min = " << min << "; max = " << max << '\n';

            throw std::runtime_error {err_msg.str()};
        }

        return (max - min) / static_cast<FP>(n_bins);
    }
};

}  // namespace mathtools
