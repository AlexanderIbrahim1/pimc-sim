#pragma once

#include <algorithm>
#include <array>
#include <filesystem>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <torch/script.h>

#include <coordinates/attard.hpp>
#include <interactions/four_body/constants.hpp>
#include <interactions/four_body/interaction_ranges.hpp>
#include <interactions/four_body/long_range.hpp>
#include <interactions/four_body/rescaling.hpp>
#include <interactions/four_body/short_range.hpp>
#include <interactions/four_body/transformers.hpp>

// TODO: add a compile-time switch to determine which of the input transformers to use in the
// final published model

namespace interact
{

enum class PermutationTransformerFlag
{
    EXACT,
    APPROXIMATE
};

}  // namespace interact

namespace impl_interact_published_model
{

inline auto get_rescaling_energy_model(const std::filesystem::path& rescaled_module_path)
    -> interact::rescale::RescalingEnergyModel<float>
{
    auto rescaled_module = torch::jit::load(rescaled_module_path.string());

    auto rescaling_potential = interact::rescale::RescalingFunction<float> {
        interact::constants4b::RESCALING_EXPON_COEFF<float>,
        interact::constants4b::RESCALING_EXPON_DECAY<float>,
        interact::constants4b::RESCALING_DISP_COEFF<float>};

    auto reverse_rescaling_limits = interact::rescale::RescalingLimits<float> {
        interact::constants4b::REVERSE_RESCALING_LIMITS_TO_LEFT<float>,
        interact::constants4b::REVERSE_RESCALING_LIMITS_TO_RIGHT<float>,
        interact::constants4b::REVERSE_RESCALING_LIMITS_FROM_LEFT<float>,
        interact::constants4b::REVERSE_RESCALING_LIMITS_FROM_RIGHT<float>};

    auto reverse_energy_rescaler =
        interact::rescale::ReverseEnergyRescaler {rescaling_potential, reverse_rescaling_limits};

    const auto rescaling_energy_model =
        interact::rescale::RescalingEnergyModel<float> {std::move(rescaled_module), reverse_energy_rescaler};

    return rescaling_energy_model;
}

template <interact::PermutationTransformerFlag Flag>
auto get_transformer()
{
    using PTF = interact::PermutationTransformerFlag;

    const auto reciprocal_factor_transformer =
        interact::trans::ReciprocalFactorTransformer<float> {interact::constants4b::MIN_SIDELENGTH<float>};

    if constexpr (Flag == PTF::EXACT) {
        const auto permutation_transformer = interact::trans::MinimumPermutationTransformer<float> {};

        return interact::trans::SampleTransformer<float> {
            std::move(reciprocal_factor_transformer), std::move(permutation_transformer)};
    }
    else {
        const auto permutation_transformer = interact::trans::ApproximateMinimumPermutationTransformer<float> {};

        return interact::trans::ApproximateSampleTransformer<float> {
            std::move(reciprocal_factor_transformer), std::move(permutation_transformer)};
    }
}

}  // namespace impl_interact_published_model
