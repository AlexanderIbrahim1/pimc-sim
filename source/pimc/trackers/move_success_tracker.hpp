#pragma once

#include <cstdint>

namespace pimc
{

class MoveSuccessTracker
{
public:
    constexpr void add_accept(std::uint64_t n_accept = 1) noexcept
    {
        n_total_accept_ += n_accept;
    }

    constexpr auto get_accept() const noexcept
    {
        return n_total_accept_;
    }

    constexpr void add_reject(std::uint64_t n_reject = 1) noexcept
    {
        n_total_reject_ += n_reject;
    }

    constexpr auto get_reject() const noexcept
    {
        return n_total_reject_;
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

}  // namespace pimc
