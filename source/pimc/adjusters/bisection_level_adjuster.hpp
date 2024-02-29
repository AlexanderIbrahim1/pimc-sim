#pragma once

#include <concepts>
#include <cstddef>

#include <pimc/adjusters/adjust_common.hpp>
#include <pimc/bisection_level_move_info.hpp>
#include <pimc/trackers/move_success_tracker.hpp>

/*
PLAN
- no DirectionIfAcceptTooLow
  - we already know the direction
- no MoveLimits
  - we already know the limits
- still need AcceptPercentageRange
- still need abs_adjustment
- still need NoMovesPolicy
*/

namespace pimc
{

template <std::floating_point FP>
class BisectionLevelMoveAdjuster
{
public:
    explicit BisectionLevelMoveAdjuster(
        AcceptPercentageRange<FP> accept_percent_range,
        FP abs_adjustment,
        NoMovesPolicy policy = NoMovesPolicy::DO_NOTHING
    )
        : accept_percent_range_ {accept_percent_range}
        , abs_adjustment_ {abs_adjustment}
        , policy_ {policy}
    {
        ctr_check_abs_adjustment_positive_and_bounded_(abs_adjustment_);
    }

    constexpr auto adjust_step(BisectionLevelMoveInfo<FP> current, const MoveSuccessTracker& move_tracker) const
        -> BisectionLevelMoveInfo<FP>
    {
        // NOTE: this is duplicated between BisectionLevelMoveAdjuster and SingleValueMoveAdjuster, but the
        // possibility of an early return complicates moving it into its own function
        const auto accept_ratio = acceptance_ratio<FP>(move_tracker);

        if (!accept_ratio.has_value()) {
            if (policy_ == NoMovesPolicy::DO_NOTHING) {
                return current;
            }
            else {
                throw std::runtime_error {
                    "ERROR: no moves were made, and no information is available to adjust the moves.\n"};
            }
        }

        auto new_upper_level_frac = FP {};
        auto new_lower_level = std::uint64_t {};

        if (*accept_ratio < accept_percent_range_.lower_accept_percentage()) {
            // make smaller steps by decreasing the fraction of upper level steps
            new_upper_level_frac = current.upper_level_frac - abs_adjustment_;
            new_lower_level = current.lower_level;

            if (new_upper_level_frac < FP {0.0}) {
                new_upper_level_frac += FP {1.0};
                new_lower_level -= 1;
            }
        }
        else if (*accept_ratio > accept_percent_range_.upper_accept_percentage()) {
            // make larger steps by increasing the fraction of upper level steps
            new_upper_level_frac = current.upper_level_frac + abs_adjustment_;
            new_lower_level = current.lower_level;

            if (new_upper_level_frac > FP {1.0}) {
                new_upper_level_frac -= FP {1.0};
                new_lower_level += 1;
            }
        }
        else {
            new_upper_level_frac = current.upper_level_frac;
            new_lower_level = current.lower_level;
        }

        if (new_lower_level == 0) {
            new_upper_level_frac = FP {0.0};
            new_lower_level = 1;
        }

        return BisectionLevelMoveInfo<FP> {new_upper_level_frac, new_lower_level};
    }

private:
    AcceptPercentageRange<FP> accept_percent_range_;
    FP abs_adjustment_;
    NoMovesPolicy policy_;
    std::size_t lowest_level_;

    // TODO: move this elsewhere after I get it to work; it is duplicated between SingleValueMoveAdjuster
    // and BisectionLevelMoveAdjuster
    void ctr_check_abs_adjustment_positive_and_bounded_(FP value) const
    {
        if (value <= FP {0.0}) {
            auto err_msg = std::stringstream {};
            err_msg << "The move adjustment must be positive.\n";
            err_msg << "Found: " << std::fixed << std::setprecision(8) << value << '\n';
            throw std::runtime_error {err_msg.str()};
        }

        if (value >= FP {1.0}) {
            auto err_msg = std::stringstream {};
            err_msg << "The move adjustment must be less than 1; cannot hop over more than one level.\n";
            err_msg << "Found: " << std::fixed << std::setprecision(8) << value << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }
};

}  // namespace pimc
