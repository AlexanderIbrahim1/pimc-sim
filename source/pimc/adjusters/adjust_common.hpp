#pragma once

#include <concepts>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>

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

template <std::floating_point FP>
struct MoveLimits
{
public:
    MoveLimits(std::optional<FP> lower, std::optional<FP> upper)
        : lower_ {lower}
        , upper_ {upper}
    {
        if (lower_.has_value() && upper_.has_value()) {
            ctr_check_value_order(lower_.value(), upper_.value());
        }
    }

    constexpr auto lower() const noexcept -> std::optional<FP>
    {
        return lower_;
    }

    constexpr auto upper() const noexcept -> std::optional<FP>
    {
        return upper_;
    }

private:
    std::optional<FP> lower_;
    std::optional<FP> upper_;

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
struct AcceptPercentageRange
{
public:
    AcceptPercentageRange(FP lower_accept_percentage, FP upper_accept_percentage)
        : lower_accept_percentage_ {lower_accept_percentage}
        , upper_accept_percentage_ {upper_accept_percentage}
    {
        ctr_check_accept_percentage_range(lower_accept_percentage_, "lower_accept_percentage");
        ctr_check_accept_percentage_range(upper_accept_percentage_, "upper_accept_percentage");
        ctr_check_percentage_order(lower_accept_percentage_, upper_accept_percentage_);
    }

    constexpr auto lower_accept_percentage() const noexcept -> FP
    {
        return lower_accept_percentage_;
    }

    constexpr auto upper_accept_percentage() const noexcept -> FP
    {
        return upper_accept_percentage_;
    }

private:
    FP lower_accept_percentage_;
    FP upper_accept_percentage_;

    void ctr_check_accept_percentage_range(FP percentage, std::string_view name) const
    {
        if (percentage < FP {0.0} || percentage > FP {1.0}) {
            auto err_msg = std::stringstream {};
            err_msg << "The '" << name << "' percentage must be between 0.0 and 1.0, inclusive.\n";
            err_msg << "Found: " << std::fixed << std::setprecision(8) << percentage << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }

    void ctr_check_percentage_order(FP lower, FP upper) const
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

}  // namespace pimc
