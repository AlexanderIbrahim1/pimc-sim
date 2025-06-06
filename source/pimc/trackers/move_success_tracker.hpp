#pragma once

#include <cstdint>
#include <optional>
#include <tuple>

namespace pimc
{

class MoveSuccessTracker
{
public:
    constexpr void add_accept(std::uint64_t n_accept = 1) noexcept
    {
        n_total_accept_ += n_accept;
    }

    constexpr void add_reject(std::uint64_t n_reject = 1) noexcept
    {
        n_total_reject_ += n_reject;
    }

    constexpr auto get_accept() const noexcept -> std::uint64_t
    {
        return n_total_accept_;
    }

    constexpr auto get_reject() const noexcept -> std::uint64_t
    {
        return n_total_reject_;
    }

    constexpr auto get_accept_and_reject() const noexcept -> std::tuple<std::uint64_t, std::uint64_t>
    {
        return {n_total_accept_, n_total_reject_};
    }

    constexpr auto get_total_attempts() const noexcept -> std::uint64_t
    {
        return n_total_accept_ + n_total_reject_;
    }

    constexpr void reset() noexcept
    {
        n_total_accept_ = 0;
        n_total_reject_ = 0;
    }

private:
    std::uint64_t n_total_accept_ {};
    std::uint64_t n_total_reject_ {};
};

template <std::floating_point FP>
constexpr auto acceptance_ratio(const MoveSuccessTracker& move_tracker) noexcept -> std::optional<FP>
{
    if (move_tracker.get_total_attempts() == 0) {
        return std::nullopt;
    }

    const auto n_accept = static_cast<FP>(move_tracker.get_accept());
    const auto n_total = static_cast<FP>(move_tracker.get_total_attempts());

    return n_accept / n_total;
}

}  // namespace pimc
