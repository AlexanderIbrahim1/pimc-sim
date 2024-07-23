#pragma once

#include <algorithm>
#include <tuple>

#include <torch/script.h>

#include <interactions/four_body/interaction_ranges.hpp>
#include <interactions/four_body/long_range.hpp>
#include <interactions/four_body/rescaling.hpp>
#include <interactions/four_body/short_range.hpp>
#include <interactions/four_body/transformers.hpp>


namespace interact {

/*
TODO: finish the proper porting; this almost certainly does not compile yet
*/


template <std::floating_point FP>
class ExtrapolatedPotential {
public:
    using RescalingModel = typename rescale::RescalingEnergyModel<FP>;
    using SampleTransformer = typename trans::SampleTransformer<FP>;
    using LongRangeEnergyCorrector = typename interact::LongRangeEnergyCorrector<FP>;
    using InteractionRangeClassifier = typename interact::InteractionRangeClassifier<FP>;
    using ShortRangeDataPreparer = typename interact::ShortRangeDataPreparer<FP>;
    using ShortRangeEnergyCorrector = typename interact::ShortRangeEnergyCorrector<FP>;
    using IR = interact::InteractionRange;
    using DistanceInfo = typename interact::ExtrapolationDistanceInfo<FP>;
    using ExtrapolationEnergies = typename interact::ExtrapolationEnergies<FP>;

    explicit ExtrapolatedPotential(
        RescalingModel rescaling_model,
        SampleTransformer transformer,
        LongRangeEnergyCorrector long_range_corrector,
        InteractionRangeClassifier interaction_range_classifier,
        ShortRangeDataPreparer short_range_preparer,
        ShortRangeEnergyCorrector short_range_corrector
    )
        : rescaling_model_ {rescaling_model}
        , transformer_ {transformer}
        , long_range_corrector_ {long_range_corrector}
        , interaction_range_classifier_ {interaction_range_classifier}
        , short_range_preparer_ {short_range_preparer}
        , short_range_corrector_ {short_range_corrector}
    {}

    constexpr auto evaluate_batch(const torch::Tensor& samples) const -> torch::Tensor {
        const auto interaction_ranges = assign_interaction_ranges_(samples);

        const auto& [batch_sidelengths, distance_infos] = fill_batch_sidelengths_and_distance_infos_(
            interaction_ranges, samples
        );

        auto transformed_batch_sidelengths = batch_sidelengths.clone();
        transform_batch_sidelengths_(transformed_batch_sidelengths);

        const auto batch_energies = rescaling_model_(transformed_batch_sidelengths, batch_sidelengths);

        auto output_energies = torch::empty({samples.size(0), 1});
        long int i_batch {};
        std::size_t i_dist_info {};

        for (long int i_sample {}; i_sample < samples.size(0); ++i_sample) {
            const auto irange = interaction_ranges[static_cast<std::size_t>(i_sample)];

            if (irange == IR::short_range) {
                const auto dist_info = distance_infos[i_dist_info++];
                const auto lower_energy = batch_energies[i_batch++].template item<FP>();
                const auto upper_energy = batch_energies[i_batch++].template item<FP>();
                const auto extrap_energies = ExtrapolationEnergies {lower_energy, upper_energy};
                output_energies[i_sample] = short_range_corrector_(extrap_energies, dist_info);
            }
            else if (irange == IR::mid_range) {
                output_energies[i_sample] = batch_energies[i_batch++].template item<FP>();
            }
            else if (irange == IR::mixed_mid_long_range) {
                const auto abinitio_energy = batch_energies[i_batch++].template item<FP>();
                output_energies[i_sample] = long_range_corrector_.mixed(abinitio_energy, samples[i_sample]); 
            }
            else {
                output_energies[i_sample] = long_range_corrector_.dispersion(samples[i_sample]);
            }
        }

        return output_energies;
    }

private:
    RescalingModel rescaling_model_;
    SampleTransformer transformer_;
    LongRangeEnergyCorrector long_range_corrector_;
    InteractionRangeClassifier interaction_range_classifier_;
    ShortRangeDataPreparer short_range_preparer_;
    ShortRangeEnergyCorrector short_range_corrector_;

    constexpr auto assign_interaction_ranges_(const torch::Tensor& samples) const -> std::vector<IR> {
        const long int n_samples = samples.size(0);
        const long int sample_size = samples.size(1);

        auto interaction_ranges = std::vector<interact::InteractionRange> {};
        interaction_ranges.reserve(static_cast<std::size_t>(n_samples));

        for (long int i = 0; i < n_samples; ++i) {
            const auto* sample_begin = samples.data_ptr<FP>() + i * sample_size;
            const auto* sample_end = sample_begin + sample_size;

            const auto irange = interaction_range_classifier_.classify(sample_begin, sample_end);
            interaction_ranges.push_back(irange);
        }

        return interaction_ranges;
    }

    constexpr auto reserved_distance_infos_(const std::vector<IR>& interaction_ranges)
        const -> std::vector<DistanceInfo>
    {
        const long int n_short_range_samples = std::count_if(
            interaction_ranges.begin(),
            interaction_ranges.end(),
            [](auto irange) { return irange == IR::short_range; }
        );

        auto distance_infos = std::vector<DistanceInfo> {};
        distance_infos.reserve(static_cast<std::size_t>(n_short_range_samples));

        return distance_infos;
    }

    constexpr auto total_batch_size_(const std::vector<IR>& interaction_ranges) const -> long int {
        return std::accumulate(interaction_ranges.begin(), interaction_ranges.end(), 0,
            [](long int accumulator, IR irange) {
                return accumulator + interact::interaction_range_size_allocation(irange);
            }
        );
    }

    constexpr auto fill_batch_sidelengths_and_distance_infos_(
        const std::vector<IR>& interaction_ranges,
        const torch::Tensor& input_sidelengths
    ) const -> std::tuple<torch::Tensor, std::vector<DistanceInfo>>
    {
        auto distance_infos = reserved_distance_infos_(interaction_ranges);
        auto batch_sidelengths = torch::empty(
            {total_batch_size_(interaction_ranges), 6},
            torch::TensorOptions().dtype(torch::CppTypeToScalarType<FP>())
        );

        long int i_batch {};

        for (std::size_t i_sample {}; i_sample < interaction_ranges.size(); ++i_sample) {
            const auto irange = interaction_ranges[i_sample];
            const auto sample = input_sidelengths[static_cast<long int>(i_sample)];

            if (irange == IR::short_range) {
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

                distance_infos.push_back(dist_info);
            }
            else if (irange == IR::mid_range || irange == IR::mixed_mid_long_range) {
                batch_sidelengths[i_batch++] = torch::from_blob(sample.template data_ptr<FP>(), {6});
            }
        }

        return {batch_sidelengths, distance_infos};
    }

    constexpr auto transform_batch_sidelengths_(torch::Tensor& batch_sidelengths) const -> void {
        const long int batch_size = batch_sidelengths.size(0);
        const long int sample_size = batch_sidelengths.size(1);

        for (long int i_batch {}; i_batch < batch_size; ++i_batch) {
            auto sample = batch_sidelengths[i_batch];
            sample = sample.view({sample_size});
            transformer_(sample);
        }
    }
};


} // namespace pot
