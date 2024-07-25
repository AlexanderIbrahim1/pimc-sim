#pragma once

#include <algorithm>
#include <array>
#include <tuple>

#include <torch/script.h>

#include <interactions/four_body/constants.hpp>
#include <interactions/four_body/interaction_ranges.hpp>
#include <interactions/four_body/long_range.hpp>
#include <interactions/four_body/rescaling.hpp>
#include <interactions/four_body/short_range.hpp>
#include <interactions/four_body/transformers.hpp>

namespace interact
{

/*
TODO: finish the proper porting; this almost certainly does not compile yet
*/

template <std::floating_point FP>
class ExtrapolatedPotential
{
public:
    using IR = interact_ranges::InteractionRange;
    using RescalingModel = typename rescale::RescalingEnergyModel<FP>;
    using SampleTransformer = typename trans::SampleTransformer<FP>;
    using LongRangeEnergyCorrector = typename long_range::LongRangeEnergyCorrector<FP>;
    using ShortRangeDataPreparer = typename short_range::ShortRangeDataPreparer<FP>;
    using ShortRangeEnergyCorrector = typename short_range::ShortRangeEnergyCorrector<FP>;
    using DistanceInfo = typename short_range::ExtrapolationDistanceInfo<FP>;
    using ExtrapolationEnergies = typename short_range::ExtrapolationEnergies<FP>;

    ExtrapolatedPotential(
        RescalingModel rescaling_model,
        SampleTransformer transformer,
        LongRangeEnergyCorrector long_range_corrector,
        ShortRangeDataPreparer short_range_preparer,
        ShortRangeEnergyCorrector short_range_corrector
    )
        : rescaling_model_ {rescaling_model}
        , transformer_ {transformer}
        , long_range_corrector_ {long_range_corrector}
        , short_range_preparer_ {short_range_preparer}
        , short_range_corrector_ {short_range_corrector}
    {}

    constexpr auto evaluate_batch(const torch::Tensor& samples) const -> torch::Tensor
    {
        const auto interaction_ranges = assign_interaction_ranges_(samples);

        const auto& [batch_sidelengths, distance_infos] =
            fill_batch_sidelengths_and_distance_infos_(interaction_ranges, samples);

        auto transformed_batch_sidelengths = batch_sidelengths.clone();
        transform_batch_sidelengths_(transformed_batch_sidelengths);

        const auto batch_energies = rescaling_model_(transformed_batch_sidelengths, batch_sidelengths);

        auto output_energies = torch::empty({samples.size(0), 1});
        long int i_batch {};
        std::size_t i_dist_info {};

        const auto calculate_short_range_energy = [&i_dist_info, &i_batch]()
        {
            const auto dist_info = distance_infos[i_dist_info++];
            const auto lower_energy = batch_energies[i_batch++].template item<FP>();
            const auto upper_energy = batch_energies[i_batch++].template item<FP>();
            const auto extrap_energies = ExtrapolationEnergies {lower_energy, upper_energy};

            return short_range_corrector_(extrap_energies, dist_info);
        };

        const auto calculate_mid_range_energy = [&i_batch]() { return batch_energies[i_batch++].template item<FP>(); };

        const auto calculate_shortmid_range_energy = [&i_dist_info, &i_batch](const torch::Tensor& sample)
        {
            constexpr auto lower = constants4b::LOWER_SHORT_DISTANCE<FP>;
            constexpr auto upper = constants4b::UPPER_SHORT_DISTANCE<FP>;
            const auto short_range_energy = calculate_short_range_energy();
            const auto mid_range_energy = calculate_mid_range_energy();

            const auto mid_side_length = torch::min(sample).template item<FP>();

            const auto fraction_mid = common_utils::smooth_01_transition(min_side_length, lower, upper);
            const auto fraction_short = FP{1.0} - fraction_mid;

            return fraction_short * short_range_energy + fraction_mid * mid_range_energy;
        };

        const auto calculate_mixed_range_energy = [&i_batch](FP abinitio_energy, const torch::Tensor& sample) {
            return long_range_corrector_.mixed(abinitio_energy, sample);
        };

        const auto calculate_long_range_energy = [](const torch::Tensor& sample) {
            long_range_corrector_.dispersion(sample);
        };

        for (long int i_sample {}; i_sample < samples.size(0); ++i_sample) {
            const auto irange = interaction_ranges[static_cast<std::size_t>(i_sample)];

            if (irange == IR::ABINITIO_SHORT) {
                output_energies[i_sample] = calculate_short_range_energy();
            } else
            if (irange == IR::ABINITIO_SHORTMID) {
                const auto sample = samples[i_sample];
                output_energies[i_sample] = calculate_shortmid_range_energy(sample);
            } else
            if (irange == IR::ABINITIO_MID) {
                output_energies[i_sample] = calculate_mid_range_energy();
            } else
            if (irange == IR::MIXED_SHORT) {
                const auto sample = samples[i_sample];
                const abinitio_energy = calculate_short_range_energy();
                output_energies[i_sample] = calculate_mixed_range_energy(abinitio_energy, sample);
            } else
            if (irange == IR::MIXED_SHORTMID) {
                const auto sample = samples[i_sample];
                const abinitio_energy = calculate_shortmid_range_energy(sample);
                output_energies[i_sample] = calculate_mixed_range_energy(abinitio_energy, sample);
            } else
            if (irange == IR::MIXED_MID) {
                const auto sample = samples[i_sample];
                const abinitio_energy = calculate_mid_range_energy();
                output_energies[i_sample] = calculate_mixed_range_energy(abinitio_energy, sample);
            } else {
                const auto sample = samples[i_sample];
                output_energies[i_sample] = calculate_long_range_energy(sample)
            }
        }

        return output_energies;
    }

private:
    RescalingModel rescaling_model_;
    SampleTransformer transformer_;
    LongRangeEnergyCorrector long_range_corrector_;
    ShortRangeDataPreparer short_range_preparer_;
    ShortRangeEnergyCorrector short_range_corrector_;

    constexpr auto assign_interaction_ranges_(const torch::Tensor& samples) const -> std::vector<IR>
    {
        const long int n_samples = samples.size(0);
        const long int sample_size = samples.size(1);

        auto interaction_ranges = std::vector<interact::InteractionRange> {};
        interaction_ranges.reserve(static_cast<std::size_t>(n_samples));

        for (long int i = 0; i < n_samples; ++i) {
            const auto* sample_begin = samples.data_ptr<FP>() + i * sample_size;
            const auto* sample_end = sample_begin + sample_size;

            const auto irange = interact_range::classify_interaction_range(sample_begin, sample_end);
            interaction_ranges.push_back(irange);
        }

        return interaction_ranges;
    }

    constexpr auto reserved_distance_infos_(const std::vector<IR>& interaction_ranges) const
        -> std::vector<DistanceInfo>
    {
        const long int n_short_range_samples = std::count_if(
            interaction_ranges.begin(),
            interaction_ranges.end(),
            [](auto irange) { return interact_ranges::is_partly_short(irange); }
        );

        auto distance_infos = std::vector<DistanceInfo> {};
        distance_infos.reserve(static_cast<std::size_t>(n_short_range_samples));

        return distance_infos;
    }

    constexpr auto total_batch_size_(const std::vector<IR>& interaction_ranges) const -> long int
    {
        return std::accumulate(
            interaction_ranges.begin(),
            interaction_ranges.end(),
            0,
            [](long int accumulator, IR irange)
            { return accumulator + interact_ranges::interaction_range_size_allocation(irange); }
        );
    }

    constexpr auto fill_batch_sidelengths_and_distance_infos_(
        const std::vector<IR>& interaction_ranges,
        const torch::Tensor& input_sidelengths
    ) const -> std::tuple<torch::Tensor, std::vector<DistanceInfo>>
    {
        auto distance_infos = reserved_distance_infos_(interaction_ranges);
        auto batch_sidelengths = torch::empty(
            {total_batch_size_(interaction_ranges), 6}, torch::TensorOptions().dtype(torch::CppTypeToScalarType<FP>())
        );

        long int i_batch {};

        const auto push_short_sample = [&batch_sidelengths, &i_batch](const torch::Tensor& sample)
        {
            const auto* begin = sample.data_ptr<FP>();
            const auto* end = begin + sample.numel();
            const auto& [extrap_sidelengths, dist_info] = short_range_preparer_.prepare(begin, end);
            const auto& [lower, upper] = extrap_sidelengths;

            const auto lower_batch_ptr = batch_sidelengths[i_batch].template data_ptr<FP>();
            std::copy(lower.begin(), lower.end(), lower_batch_ptr);
            ++i_batch;

            const auto upper_batch_ptr = batch_sidelengths[i_batch].template data_ptr<FP>();
            std::copy(upper.begin(), upper.end(), upper_batch_ptr);
            ++i_batch;
        };

        const auto push_unmodified_sample = [&batch_sidelengths, &i_batch](const torch::Tensor& sample)
        { batch_sidelengths[i_batch++] = torch::from_blob(sample.template data_ptr<FP>(), {6}); };

        for (std::size_t i_sample {}; i_sample < interaction_ranges.size(); ++i_sample) {
            const auto irange = interaction_ranges[i_sample];
            const auto sample = input_sidelengths[static_cast<long int>(i_sample)];

            if (irange == IR::ABINITIO_SHORT || ir == IR::MIXED_SHORT) {
                push_short_sample(sample);
            }
            else if (irange == IR::ABINITIO_SHORTMID || ir == IR::MIXED_SHORTMID) {
                push_short_sample(sample);
                push_unmodified_sample(sample);
            }
            else if (irange == IR::ABINITIO_MID || irange == IR::MIXED_MID) {
                push_unmodified_sample(sample);
            }
            else {
                // IR::LONG (do nothing)
            }
        }

        return {batch_sidelengths, distance_infos};
    }

    constexpr auto transform_batch_sidelengths_(torch::Tensor& batch_sidelengths) const -> void
    {
        const long int batch_size = batch_sidelengths.size(0);
        const long int sample_size = batch_sidelengths.size(1);

        for (long int i_batch {}; i_batch < batch_size; ++i_batch) {
            auto sample = batch_sidelengths[i_batch];
            sample = sample.view({sample_size});
            transformer_(sample);
        }
    }
};

}  // namespace interact
