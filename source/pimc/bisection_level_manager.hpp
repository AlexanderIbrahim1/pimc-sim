#pragma once

#include <cstddef>
#include <iterator>
#include <span>
#include <vector>


namespace pimc {

struct BisectionTriplets {
    std::size_t left {};
    std::size_t mid {};
    std::size_t right {};

    constexpr auto operator==(const BisectionTriplets& other) const noexcept -> bool {
        return (left == other.left && mid == other.mid && right == other.right);
    }
};

static constexpr auto pow_int(std::size_t base, std::size_t exponent) noexcept -> std::size_t {
    auto result = std::size_t {1};
    for (std::size_t i {0}; i < exponent; ++i) {
        result *= base;
    }
    return result;
}

class BisectionLevelManager {
public:
    BisectionLevelManager() = delete;

    constexpr explicit BisectionLevelManager(std::size_t max_level)
        : max_level_ {max_level}
    {
        const auto max_number_of_triplets = pow_int(2, max_level_) - 1;
        indices_.reserve(max_number_of_triplets);

        const auto max_right_endpoint = pow_int(2, max_level_);
        indices_.emplace_back(BisectionTriplets{0, max_right_endpoint/2, max_right_endpoint});

        for (std::size_t i {0}; i < max_number_of_triplets; ++i) {
            const auto current = indices_[i];

            if (current.mid - current.left == 1) {
                continue;
            }

            const auto left_triplet_midpoint = (current.mid + current.left) / 2;
            const auto right_triplet_midpoint = (current.mid + current.right) / 2;

            indices_.emplace_back(BisectionTriplets{current.left, left_triplet_midpoint, current.mid});
            indices_.emplace_back(BisectionTriplets{current.mid, right_triplet_midpoint, current.right});
        }
    }

    constexpr auto triplets(std::size_t level) const noexcept -> std::span<const BisectionTriplets> {
        const auto left_offset = static_cast<std::ptrdiff_t>(pow_int(2, level) - 1);
        const auto right_offset = static_cast<std::ptrdiff_t>(pow_int(2, level + 1) - 1);

        const auto left = std::next(std::begin(indices_), left_offset);
        const auto right = std::next(std::begin(indices_), right_offset);

        return std::span {left, right};
    }

private:
    std::size_t max_level_;
    std::vector<BisectionTriplets> indices_;
};

}
