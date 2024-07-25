#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <numeric>

#include <interactions/four_body/constants.hpp>

namespace interact
{

namespace interact_ranges
{

template <std::floating_point FP>
constexpr auto mean_of_six(FP const* const begin, FP const* const end) -> FP
{
    const auto add = [](FP x, FP y) { return x + y; };
    auto total = std::accumulate(begin, end, add, FP {});
    return total / FP {6.0};
}

enum class InteractionRange
{
    ABINITIO_SHORT,
    ABINITIO_SHORTMID,
    ABINITIO_MID,
    MIXED_SHORT,
    MIXED_SHORTMID,
    MIXED_MID,
    LONG
};

template <std::floating_point FP>
constexpr auto classify_interaction_range(const std::array<FP, 6>& side_lengths) noexcept -> InteractionRange
{
    return classify_interaction_range(side_lengths.begin(), side_lengths.end());
}

template <std::floating_point FP>
constexpr auto classify_interaction_range(const FP* begin, const FP* end) noexcept -> InteractionRange
{
    using IR = InteractionRange;

    const auto average_side_length = mean_of_six(begin, end);

    if (average_side_length > constants4b::UPPER_MIXED_DISTANCE<FP>) {
        return IR::LONG;
    }

    const auto is_abinitio = [&](FP x) { return x < constants4b::LOWER_MIXED_DISTANCE<FP>; };
    const auto is_short = [&](const FP* begin_, const FP* end_)
    { return std::any_of(begin_, end_, [](FP x) { return x < constants4b::LOWER_SHORT_DISTANCE<FP>; }) };
    const auto is_shortmid = [&](const FP* begin_, const FP* end_)
    {
        return std::any_of(
            begin_,
            end_,
            [](FP x) { return constants4b::LOWER_SHORT_DISTANCE<FP> <= x && x < constants4b::UPPER_SHORT_DISTANCE<FP>; }
        )
    };

    if (is_abinitio(average_side_length)) {
        if (is_short(begin, end)) {
            return IR::ABINITIO_SHORT;
        }
        else if (is_shortmid(begin, end)) {
            return IR::ABINITIO_SHORTMID;
        }
        else {
            return IR::ABINITIO_MID;
        }
    }
    else {
        if (is_short(begin, end)) {
            return IR::MIXED_SHORT;
        }
        else if (is_shortmid(begin, end)) {
            return IR::MIXED_SHORTMID;
        }
        else {
            return IR::MIXED_MID;
        }
    }
}

constexpr auto interaction_range_size_allocation(InteractionRange ir) noexcept -> long int
{
    using IR = InteractionRange;

    if (ir == IR::LONG) {
        return 0;
    }
    else if (ir == IR::ABINITIO_MID || ir == IR::MIXED_MID) {
        return 1;
    }
    else if (ir == IR::ABINITIO_SHORT || ir == IR::MIXED_SHORT) {
        return 2;
    }
    else {
        return 3;
    }
}

constexpr auto is_partly_short(InteractionRange ir) noexcept -> bool
{
    using IR = InteractionRange;

    return (
        ir == IR::ABINITIO_SHORT || ir == IR::ABINITIO_SHORTMID || ir == IR::MIXED_SHORT || ir == IR::MIXED_SHORTMID
    );
}

}  // namespace interact_ranges

}  // namespace interact
