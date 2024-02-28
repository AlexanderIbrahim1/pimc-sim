#pragma once

#include <algorithm>
#include <concepts>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>

#include <pimc/adjusters/adjust_common.hpp>
#include <pimc/trackers/move_success_tracker.hpp>

namespace pimc
{

template <std::floating_point FP>
class SingleValueMoveAdjuster
{
public:
    explicit SingleValueMoveAdjuster(
        AcceptPercentageRange<FP> accept_percent_range,
        FP abs_adjustment,
        DirectionIfAcceptTooLow direction,
        std::optional<MoveLimits<FP>> move_limits = std::nullopt,
        NoMovesPolicy policy = NoMovesPolicy::DO_NOTHING
    )
        : accept_percent_range_ {accept_percent_range}
        , abs_adjustment_ {abs_adjustment}
        , direction_ {direction}
        , move_limits_ {move_limits}
        , policy_ {policy}
    {
        ctr_check_abs_adjustment_positive(abs_adjustment_);
    }

    constexpr auto adjust_step(FP current, const MoveSuccessTracker& move_tracker) const -> FP
    {
        if (move_tracker.get_total_attempts() == 0) {
            if (policy_ == NoMovesPolicy::DO_NOTHING) {
                return current;
            }
            else {
                throw std::runtime_error {
                    "ERROR: no moves were made, and no information is available to adjust the moves.\n"};
            }
        }

        const auto accept_ratio = [&move_tracker]()
        {
            const auto n_accept = static_cast<double>(move_tracker.get_accept());
            const auto n_total = static_cast<double>(move_tracker.get_total_attempts());

            return n_accept / n_total;
        }();

        auto new_step = FP {};
        if (accept_ratio < accept_percent_range_.lower_accept_percentage()) {
            if (direction_ == DirectionIfAcceptTooLow::POSITIVE) {
                new_step = current + abs_adjustment_;
            }
            else {
                new_step = current - abs_adjustment_;
            }
        }
        else if (accept_ratio > accept_percent_range_.upper_accept_percentage()) {
            // logic: if we increase the step when the acceptance percentage is too low, we
            // should decrease the step when the acceptance percentage is too high
            //
            // there might be cases where this isn't what we want, but for the moves I've seen
            // so far, this is a fairly straightforward decision to make
            if (direction_ == DirectionIfAcceptTooLow::POSITIVE) {
                new_step = current - abs_adjustment_;
            }
            else {
                new_step = current + abs_adjustment_;
            }
        }
        else {
            new_step = current;
        }

        const auto clamped_step = apply_limits_(new_step);

        return clamped_step;
    }

private:
    AcceptPercentageRange<FP> accept_percent_range_;
    FP abs_adjustment_;
    DirectionIfAcceptTooLow direction_;
    std::optional<MoveLimits<FP>> move_limits_;
    NoMovesPolicy policy_;

    constexpr auto apply_limits_(FP new_step) const noexcept -> FP
    {
        auto clamped_step = FP {new_step};

        if (!move_limits_.has_value()) {
            return clamped_step;
        }

        const auto lower = move_limits_->lower();
        if (lower.has_value()) {
            clamped_step = std::max(clamped_step, *lower);
        }

        const auto upper = move_limits_->upper();
        if (upper.has_value()) {
            clamped_step = std::min(clamped_step, *upper);
        }

        return clamped_step;
    }

    void ctr_check_abs_adjustment_positive(FP value) const
    {
        if (value <= FP {0.0}) {
            auto err_msg = std::stringstream {};
            err_msg << "The move adjustment must be positive.\n";
            err_msg << "Found: " << std::fixed << std::setprecision(8) << value << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }
};

}  // namespace pimc
