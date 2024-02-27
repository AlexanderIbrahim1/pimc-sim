#pragma once

#include <cstddef>
#include <iterator>
#include <span>
#include <vector>

namespace pimc
{

struct BisectionIndices
{
    std::size_t left {};
    std::size_t mid {};
    std::size_t right {};

    constexpr auto operator==(const BisectionIndices& other) const noexcept -> bool
    {
        return (left == other.left && mid == other.mid && right == other.right);
    }
};

static constexpr auto pow_int(std::size_t base, std::size_t exponent) noexcept -> std::size_t
{
    auto result = std::size_t {1};
    for (std::size_t i {0}; i < exponent; ++i) {
        result *= base;
    }
    return result;
}

static constexpr auto level_segment_size(std::size_t level) noexcept -> std::size_t
{
    return pow_int(2, level);
}

class BisectionLevelManager
{
public:
    BisectionLevelManager() = delete;

    constexpr explicit BisectionLevelManager(std::size_t max_level, std::size_t offset, std::size_t modulo)
        : max_level_ {max_level}
    {
        ctr_check_offset_less_than_modulo_(offset, modulo);
        ctr_check_modulo_is_positive_(modulo);

        const auto max_number_of_triplets = pow_int(2, max_level_) - 1;
        indices_.reserve(max_number_of_triplets);

        const auto max_right = pow_int(2, max_level_);
        const auto max_mid = max_right / 2;
        indices_.emplace_back(BisectionIndices {0, max_mid, max_right});

        for (std::size_t i {0}; i < max_number_of_triplets; ++i) {
            const auto current = indices_[i];

            if (current.mid - current.left == 1) {
                continue;
            }

            const auto new_left_mid = (current.mid + current.left) / 2;
            const auto new_right_mid = (current.mid + current.right) / 2;

            indices_.emplace_back(BisectionIndices {current.left, new_left_mid, current.mid});
            indices_.emplace_back(BisectionIndices {current.mid, new_right_mid, current.right});
        }

        // 2nd pass is for applying the offsets
        for (std::size_t i {0}; i < max_number_of_triplets; ++i) {
            auto& triplet = indices_[i];
            triplet.left = (triplet.left + offset) % modulo;
            triplet.mid = (triplet.mid + offset) % modulo;
            triplet.right = (triplet.right + offset) % modulo;
        }
    }

    constexpr auto triplets(std::size_t level) const noexcept -> std::span<const BisectionIndices>
    {
        const auto left_offset = static_cast<std::ptrdiff_t>(pow_int(2, level) - 1);
        const auto right_offset = static_cast<std::ptrdiff_t>(pow_int(2, level + 1) - 1);

        const auto left = std::next(std::begin(indices_), left_offset);
        const auto right = std::next(std::begin(indices_), right_offset);

        return std::span {left, right};
    }

private:
    std::size_t max_level_;
    std::vector<BisectionIndices> indices_;

    constexpr void ctr_check_offset_less_than_modulo_(std::size_t offset, std::size_t modulo) const
    {
        if (offset >= modulo) {
            throw std::runtime_error("The offset for the bisection indices must be less than the modulo.\n");
        }
    }

    constexpr void ctr_check_modulo_is_positive_(std::size_t modulo) const
    {
        if (modulo == 0) {
            throw std::runtime_error("The modulo for the bisection indices must be positive.\n");
        }
    }
};

}  // namespace pimc
