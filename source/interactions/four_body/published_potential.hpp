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
#include <interactions/four_body/extrapolated_potential.hpp>
#include <interactions/four_body/interaction_ranges.hpp>
#include <interactions/four_body/long_range.hpp>
#include <interactions/four_body/rescaling.hpp>
#include <interactions/four_body/short_range.hpp>
#include <interactions/four_body/transformers.hpp>

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

template <std::size_t NDIM>
auto get_long_range_energy_corrector()
{
    return interact::long_range::LongRangeEnergyCorrector<float, NDIM> {
        interact::disp::FourBodyDispersionPotential<float, NDIM> {interact::constants4b::BADE_COEFF_AVTZ<float>},
        interact::constants4b::LOWER_MIXED_DISTANCE<float>,
        interact::constants4b::UPPER_MIXED_DISTANCE<float>};
}

inline auto get_short_range_data_preparer()
{
    // NOTE: the second parameter is UPPER_SHORT_DISTANCE in the python code, but I think that
    // might be a mistake? I'll take a closer look at it later
    return interact::short_range::ShortRangeDataPreparer<float> {
        interact::constants4b::SHORT_RANGE_SCALING_STEP<float>, interact::constants4b::LOWER_SHORT_DISTANCE<float>};
}

inline auto get_short_range_energy_corrector()
{
    return interact::short_range::ShortRangeEnergyCorrector<float> {
        interact::constants4b::SHORT_RANGE_CORRECT_SLOPE_MIN<float>,
        interact::constants4b::SHORT_RANGE_CORRECT_SLOPE_MAX<float>};
}

}  // namespace impl_interact_published_model

namespace interact
{

template <std::size_t NDIM, interact::PermutationTransformerFlag Flag>
auto get_published_four_body_potential(const std::filesystem::path& rescaled_module_path)
{
    auto rescaling_energy_model = impl_interact_published_model::get_rescaling_energy_model(rescaled_module_path);
    auto transformer = impl_interact_published_model::get_transformer<Flag>();
    auto long_range_energy_corrector = impl_interact_published_model::get_long_range_energy_corrector<NDIM>();
    auto short_range_data_preparer = impl_interact_published_model::get_short_range_data_preparer();
    auto short_range_energy_corrector = impl_interact_published_model::get_short_range_energy_corrector();

    return interact::ExtrapolatedPotential<float, NDIM, decltype(transformer)> {
        std::move(rescaling_energy_model),
        std::move(transformer),
        std::move(long_range_energy_corrector),
        std::move(short_range_data_preparer),
        std::move(short_range_energy_corrector)};
}

}  // namespace interact
