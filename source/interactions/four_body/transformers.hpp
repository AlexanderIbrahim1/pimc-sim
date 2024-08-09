#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <tuple>

#include <torch/script.h>

/*
    TODO: the MinimumPermutationTransformer can be optimized to not have to search through all 24
    different permutations; really, only the smallest element, then the 2nd smallest element, are needed

    But I just need to get it working for now
*/

namespace interact
{

namespace trans
{

constexpr static auto g_n_permutations = std::size_t {24};

// clang-format off
constexpr static auto g_index_swap_permutations = std::array<std::size_t, 6 * 24> {
    0, 1, 2, 3, 4, 5,
    0, 2, 1, 4, 3, 5,
    0, 3, 4, 1, 2, 5,
    0, 4, 3, 2, 1, 5,
    1, 0, 2, 3, 5, 4,
    1, 2, 0, 5, 3, 4,
    1, 3, 5, 0, 2, 4,
    1, 5, 3, 2, 0, 4,
    2, 0, 1, 4, 5, 3,
    2, 1, 0, 5, 4, 3,
    2, 4, 5, 0, 1, 3,
    2, 5, 4, 1, 0, 3,
    3, 0, 4, 1, 5, 2,
    3, 1, 5, 0, 4, 2,
    3, 4, 0, 5, 1, 2,
    3, 5, 1, 4, 0, 2,
    4, 0, 3, 2, 5, 1,
    4, 2, 5, 0, 3, 1,
    4, 3, 0, 5, 2, 1,
    4, 5, 2, 3, 0, 1,
    5, 1, 3, 2, 4, 0,
    5, 2, 4, 1, 3, 0,
    5, 3, 1, 4, 2, 0,
    5, 4, 2, 3, 1, 0
};
// clang-format on

/*
    The ReciprocalFactorTransformer both calculates the reciprocal of each element, and
    multiplies it by a certain constant factor.
*/
template <std::floating_point FP>
class ReciprocalFactorTransformer
{
public:
    explicit ReciprocalFactorTransformer(FP factor)
        : factor_ {factor}
    {}

    constexpr void operator()(torch::Tensor& values) const
    {
        for (long int i {}; i < values.size(0); ++i) {
            values[i] = factor_ / values[i];
        }
    }

private:
    FP factor_;
};

template <std::floating_point FP>
class MinimumPermutationTransformer
{
public:
    constexpr void operator()(torch::Tensor& values) const
    {
        std::array<FP, 6> original_permutation;
        const auto values_begin = values.data_ptr<FP>();
        const auto values_end = values.data_ptr<FP>() + values.numel();
        std::copy(values_begin, values_end, original_permutation.begin());

        auto minimum_permuted = std::array<FP, 6>(original_permutation);

        for (std::size_t i_perm {1}; i_perm < g_n_permutations; ++i_perm) {
            const auto permuted = permute_six_side_lengths_(i_perm, original_permutation);
            minimum_permuted = std::min(minimum_permuted, permuted);
        }

        std::copy(minimum_permuted.begin(), minimum_permuted.end(), values.data_ptr<FP>());
    }

private:
    constexpr auto permute_six_side_lengths_(std::size_t i_permutation, const std::array<FP, 6>& side_lengths) const
        -> std::array<FP, 6>
    {
        std::array<FP, 6> permuted_side_lengths;
        auto i_offset = i_permutation * 6;

        for (std::size_t i {}; i < 6; ++i) {
            auto index_to_swap = g_index_swap_permutations[i_offset + i];
            permuted_side_lengths[i] = side_lengths[index_to_swap];
        }

        return permuted_side_lengths;
    }
};

// clang-format off
constexpr static auto g_second_indices = std::array<std::size_t, 4 * 6> {
    1, 2, 3, 4,
    0, 2, 3, 5,
    0, 1, 4, 5,
    0, 1, 4, 5,
    0, 2, 3, 5,
    1, 2, 3, 4
};
// clang-format on

constexpr static auto g_n_second_indices = std::size_t {4};

template <std::floating_point FP>
class ApproximateMinimumPermutationTransformer
{
public:
    constexpr void operator()(torch::Tensor& values) const
    {
        const auto i_min0 = static_cast<std::size_t>(torch::argmin(values).item<long>());
        const auto i_min1 = argmin_over_(values, i_min0);

        const auto i_perm = i_min1 + 4 * i_min0;
        permute_(values, i_perm);
    }

private:
    constexpr auto argmin_over_(const torch::Tensor& values, std::size_t i_second) const -> std::size_t
    {
        const auto offset = g_n_second_indices * i_second;
        const auto it_start = g_second_indices.cbegin() + offset;

        const auto idx_start = static_cast<long>(*it_start);
        auto min_value = values[idx_start].template item<FP>();

        auto i_enum_min = std::size_t {};
        for (std::size_t i_enum {1}; i_enum < g_n_second_indices; ++i_enum) {
            const auto it = it_start + i_enum;
            const auto idx = static_cast<long>(*it);
            const auto new_value = values[idx].template item<FP>();

            if (new_value < min_value) {
                min_value = new_value;
                i_enum_min = i_enum;
            }
        }

        return i_enum_min;
    }

    constexpr auto permute_(torch::Tensor& values, std::size_t i_perm) const -> void
    {
        auto permuted_values = std::array<FP, 6> {};

        const auto i_offset = 6 * i_perm;
        for (std::size_t i {}; i < 6; ++i) {
            const auto index_to_swap = static_cast<long>(g_index_swap_permutations[i_offset + i]);
            permuted_values[i] = values[index_to_swap].item<FP>();
        }

        std::copy(permuted_values.cbegin(), permuted_values.cend(), values.data_ptr<FP>());
    }
};

template <std::floating_point FP>
class SampleTransformer
{
public:
    explicit SampleTransformer(
        ReciprocalFactorTransformer<FP> reciprocal_factor_transformer,
        MinimumPermutationTransformer<FP> permutation_transformer
    )
        : reciprocal_factor_transformer_ {reciprocal_factor_transformer}
        , permutation_transformer_ {permutation_transformer}
    {}

    constexpr auto operator()(torch::Tensor& values) const
    {
        reciprocal_factor_transformer_(values);
        permutation_transformer_(values);
    }

private:
    ReciprocalFactorTransformer<FP> reciprocal_factor_transformer_;
    MinimumPermutationTransformer<FP> permutation_transformer_;
};

template <std::floating_point FP>
class ApproximateSampleTransformer
{
public:
    explicit ApproximateSampleTransformer(
        ReciprocalFactorTransformer<FP> reciprocal_factor_transformer,
        ApproximateMinimumPermutationTransformer<FP> permutation_transformer
    )
        : reciprocal_factor_transformer_ {reciprocal_factor_transformer}
        , permutation_transformer_ {permutation_transformer}
    {}

    constexpr auto operator()(torch::Tensor& values) const
    {
        reciprocal_factor_transformer_(values);
        permutation_transformer_(values);
    }

private:
    ReciprocalFactorTransformer<FP> reciprocal_factor_transformer_;
    ApproximateMinimumPermutationTransformer<FP> permutation_transformer_;
};

}  // namespace trans

}  // namespace interact
