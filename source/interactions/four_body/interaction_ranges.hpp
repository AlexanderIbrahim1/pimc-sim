#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <numeric>

namespace interact
{

template <std::floating_point FP>
constexpr auto mean_of_six(const std::array<FP, 6> side_lengths) -> FP
{
    const auto add = [](FP x, FP y) { return x + y; };
    auto total = std::accumulate(side_lengths.begin(), side_lengths.end(), add, FP {});
    return total / FP {6.0};
}

template <std::floating_point FP>
struct InteractionCutoffDistances
{
    FP lower_short_distance;
    FP upper_short_distance;
    FP lower_mixed_distance;
    FP upper_mixed_distance;
};

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
constexpr auto classify_interaction_range(const std::array<FP, 6>& side_lengths, const InteractionCutoffDistances<FP>& cutoffs) noexcept -> InteractionRange {
    using IR = InteractionRange;

    const auto average_side_length = mean_of_six(side_lengths);

    if (average_side_length > cutoffs.upper_mixed_distance) {
        return IR::LONG;
    }

    const auto is_abinitio = [&](FP x) { return x < cutoffs.lower_mixed_distance; };
    const auto is_short = [&](const std::array<FP, 6>& side_lengths_) {
        return std::any_of(side_lengths_.begin(), side_lengths_.end(), 
            [](FP x) { return x < cutoffs.lower_short_distance; })
    };
    const auto is_shortmid = [&](const std::array<FP, 6>& side_lengths_) {
        return std::any_of(side_lengths_.begin(), side_lengths_.end(),
            [](FP x) { return cutoffs.lower_short_distance <= x && x < cutoffs.upper_short_distance; })
    };

    if (is_abinitio(average_side_length)) {
        if (is_short(side_lengths)) {
            return IR::ABINITIO_SHORT;
        } else
        if (is_shortmid(side_lengths)) {
            return IR::ABINITIO_SHORTMID;
        } else
        {
            return IR::ABINITIO_MID;
        }
    } else {
        if (is_short(side_lengths)) {
            return IR::MIXED_SHORT;
        } else
        if (is_shortmid(side_lengths)) {
            return IR::MIXED_SHORTMID;
        } else
        {
            return IR::MIXED_MID;
        }
    }
}

constexpr auto interaction_range_size_allocation(InteractionRange ir) noexcept -> std::size_t {
    using IR = InteractionRange;

    if (ir == IR::LONG) {
        return 0;
    } else
    if (ir == IR::ABINITIO_MID || ir == IR::MIXED_MID) {
        return 1;
    } else
    if (ir == IR::ABINITIO_SHORT || ir == IR::MIXED_SHORT) {
        return 2;
    } else
    {
        return 3;
    }
}

constexpr auto is_partly_short(InteractionRange ir) noexcept -> bool {
    using IR = InteractionRange;

    return (ir == IR::ABINITIO_SHORT || ir == IR::ABINITIO_SHORTMID || ir == IR::MIXED_SHORT || ir == IR::MIXED_SHORTMID);
}

}  // namespace interact
