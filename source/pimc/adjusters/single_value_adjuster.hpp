#pragma once

#include <concepts>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include <pimc/trackers/move_success_tracker.hpp>

namespace pimc
{

enum class NoMovesPolicy
{
    DO_NOTHING,
    THROW
};

enum class DirectionIfAcceptTooLow
{
    POSITIVE,
    NEGATIVE
};

struct AcceptPercentageRange
{
public:
    AcceptPercentageRange(
        double lower_accept_percentage,
        double upper_accept_percentage,
        DirectionIfAcceptTooLow direction
    )
        : lower_accept_percentage_ {lower_accept_percentage}
        , upper_accept_percentage_ {upper_accept_percentage}
        , direction_ {direction}
    {
        ctr_check_accept_percentage_range(lower_accept_percentage_, "lower_accept_percentage");
        ctr_check_accept_percentage_range(upper_accept_percentage_, "upper_accept_percentage");
        ctr_check_percentage_order(lower_accept_percentage_, upper_accept_percentage_);
    }

    constexpr auto lower_accept_percentage() const noexcept -> double
    {
        return lower_accept_percentage_;
    }

    constexpr auto upper_accept_percentage() const noexcept -> double
    {
        return upper_accept_percentage_;
    }

    constexpr auto direction() const noexcept -> DirectionIfAcceptTooLow
    {
        return direction_;
    }

private:
    double lower_accept_percentage_;
    double upper_accept_percentage_;
    DirectionIfAcceptTooLow direction_;

    void ctr_check_accept_percentage_range(double percentage, std::string_view name) const
    {
        if (percentage < 0.0 || percentage > 1.0) {
            auto err_msg = std::stringstream {};
            err_msg << "The '" << name << "' percentage must be between 0.0 and 1.0, inclusive.\n";
            err_msg << "Found: " << std::fixed << std::setprecision(8) << percentage << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }

    void ctr_check_percentage_order(double lower, double upper) const
    {
        if (lower >= upper) {
            auto err_msg = std::stringstream {};
            err_msg << "The lower acceptance percentage must be less than the upper acceptance percentage/\n";
            err_msg << "Found: lower = " << std::fixed << std::setprecision(8) << lower << '\n';
            err_msg << "Found: upper = " << std::fixed << std::setprecision(8) << upper << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }
};

template <std::floating_point FP>
struct MoveLimits
{
public:
    MoveLimits(FP lower, FP upper)
        : lower_ {lower}
        , upper_ {upper}
    {
        ctr_check_value_order(lower_, upper_);
    }

    constexpr auto lower() const noexcept -> FP
    {
        return lower_;
    }

    constexpr auto upper() const noexcept -> FP
    {
        return upper_;
    }

private:
    FP lower_;
    FP upper_;

    void ctr_check_value_order(double lower, double upper) const
    {
        if (lower >= upper) {
            auto err_msg = std::stringstream {};
            err_msg << "The lower value must be less than the upper value/\n";
            err_msg << "Found: lower = " << std::fixed << std::setprecision(8) << lower << '\n';
            err_msg << "Found: upper = " << std::fixed << std::setprecision(8) << upper << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }
};

template <std::floating_point FP>
class SingleValueMoveAdjuster
{
public:
    explicit SingleValueMoveAdjuster(
        AcceptPercentageRange accept_percent_range,
        MoveLimits move_limits,
        FP abs_adjustment,
        NoMovesPolicy policy = NoMovesPolicy::DO_NOTHING
    )
        : accept_percent_range_ {accept_percent_range}
        , move_limits_ {move_limits}
        , abs_adjustment_ {abs_adjustment}
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

        const auto accept_ratio = static_cast<double>(move_tracker.get_accept() / move_tracker.get_total_attempts());

        auto new_step = FP {};
        if (accept_ratio < accept_percent_range_.lower_accept_percentage()) {
            const auto direction = accept_percent_range_.direction();

            if (direction == DirectionIfAcceptTooLow::POSITIVE) {
                new_step = current + abs_adjustment_;
            }
            else {
                new_step = current - abs_adjustment_;
            }
        }
        else if (accept_ratio > accept_percent_range_.upper_accept_percentage()) {
            const auto direction = accept_percent_range_.direction();

            // logic: if we increase the step when the acceptance percentage is too low, we
            // should decrease the step when the acceptance percentage is too high
            //
            // there might be cases where this isn't what we want, but for the moves I've seen
            // so far, this is a fairly straightforward decision to make
            if (direction == DirectionIfAcceptTooLow::POSITIVE) {
                new_step = current - abs_adjustment_;
            }
            else {
                new_step = current + abs_adjustment_;
            }
        }
        else {
            new_step = current;
        }

        return new_step;
    }

private:
    AcceptPercentageRange accept_percent_range_;
    MoveLimits move_limits_;
    FP abs_adjustment_;
    NoMovesPolicy policy_;

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
