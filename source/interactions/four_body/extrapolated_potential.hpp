#pragma once

#include <iostream>

#include <algorithm>
#include <array>
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

namespace interact
{

template <std::floating_point FP, std::size_t NDIM, typename InputSampleTransformer>
class ExtrapolatedPotential
{
public:
    using IR = interact_ranges::InteractionRange;
    using RescalingModel = typename rescale::RescalingEnergyModel<FP>;
    using LongRangeEnergyCorrector = typename long_range::LongRangeEnergyCorrector<FP, NDIM>;
    using ShortRangeDataPreparer = typename short_range::ShortRangeDataPreparer<FP>;
    using ShortRangeEnergyCorrector = typename short_range::ShortRangeEnergyCorrector<FP>;
    using DistanceInfo = typename short_range::ExtrapolationDistanceInfo<FP>;
    using ExtrapolationEnergies = typename short_range::ExtrapolationEnergies<FP>;

    ExtrapolatedPotential(
        RescalingModel rescaling_model,
        InputSampleTransformer transformer,
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

        // THE DEBUGGER DOESN'T SHOW THE ELEMENTS OF THE TENSOR!!!
        //         std::cout << '\n';
        //         std::cout << "batch_sidelengths.size()" << batch_sidelengths.sizes() << '\n';
        //         std::cout << "elements:\n";
        //         for (int i = 0; i < batch_sidelengths.size(0); ++i) {
        //             for (int j = 0; j < batch_sidelengths.size(1); ++j) {
        //                 std::cout << batch_sidelengths[i][j].template item<FP>() << ' ';
        //             }
        //             std::cout << '\n';
        //         }
        //
        //         std::cout << '\n';
        //         std::cout << "Distance infos\n";
        //         std::cout << "distance_infos.size()" << distance_infos.size() << '\n';
        //         for (const auto& dinfo : distance_infos) {
        //             std::cout << "(r_short_range, r_lower, r_upper) = (";
        //             std::cout << dinfo.r_short_range << ", " << dinfo.r_lower << ", " << dinfo.r_upper << '\n';
        //         }

        auto transformed_batch_sidelengths = batch_sidelengths.clone();
        transform_batch_sidelengths_(transformed_batch_sidelengths);

        //         std::cout << '\n';
        //         std::cout << "transformed_batch_sidelengths.size()" << transformed_batch_sidelengths.sizes() << '\n';
        //         std::cout << "elements:\n";
        //         for (int i = 0; i < transformed_batch_sidelengths.size(0); ++i) {
        //             for (int j = 0; j < transformed_batch_sidelengths.size(1); ++j) {
        //                 std::cout << transformed_batch_sidelengths[i][j].template item<FP>() << ' ';
        //             }
        //             std::cout << '\n';
        //         }

        const auto batch_energies = rescaling_model_(transformed_batch_sidelengths, batch_sidelengths);

        //         std::cout << '\n';
        //         std::cout << "batch_energies.size()" << batch_energies.sizes() << '\n';
        //         std::cout << "elements:\n";
        //         for (int i = 0; i < batch_energies.size(0); ++i) {
        //             for (int j = 0; j < batch_energies.size(1); ++j) {
        //                 std::cout << batch_energies[i][j].template item<FP>() << ' ';
        //             }
        //             std::cout << '\n';
        //         }

        auto output_energies = torch::empty({samples.size(0), 1});
        long int i_batch {};
        std::size_t i_dist_info {};

        // clang-format off
        const auto calculate_short_range_energy = [&]()
        {
            const auto dist_info = distance_infos[i_dist_info++];
            const auto lower_energy = batch_energies[i_batch++].template item<FP>();
            const auto upper_energy = batch_energies[i_batch++].template item<FP>();
            const auto extrap_energies = ExtrapolationEnergies {lower_energy, upper_energy};

            return short_range_corrector_(extrap_energies, dist_info);
        };

        const auto calculate_mid_range_energy = [&]()
        {
            return batch_energies[i_batch++].template item<FP>();
        };

        const auto calculate_shortmid_range_energy = [&](const torch::Tensor& sample)
        {
            constexpr auto lower = constants4b::LOWER_SHORT_DISTANCE<FP>;
            constexpr auto upper = constants4b::UPPER_SHORT_DISTANCE<FP>;
            const auto short_range_energy = calculate_short_range_energy();
            const auto mid_range_energy = calculate_mid_range_energy();

            const auto min_side_length = torch::min(sample).template item<FP>();

            const auto fraction_mid = common_utils::smooth_01_transition(min_side_length, lower, upper);
            const auto fraction_short = FP {1.0} - fraction_mid;

            return fraction_short * short_range_energy + fraction_mid * mid_range_energy;
        };

        const auto calculate_mixed_range_energy = [&](FP abinitio_energy, const torch::Tensor& sample)
        {
            return long_range_corrector_.mixed(abinitio_energy, sample);
        };

        const auto calculate_long_range_energy = [&](const torch::Tensor& sample)
        {
            return long_range_corrector_.dispersion(sample);
        };
        // clang-format on

        for (long int i_sample {}; i_sample < samples.size(0); ++i_sample) {
            const auto irange = interaction_ranges[static_cast<std::size_t>(i_sample)];

            if (i_sample == 8) {
                std::cout << "irange of sample 8 = " << static_cast<int>(irange) << '\n';
            }

            if (irange == IR::ABINITIO_SHORT) {
                output_energies[i_sample] = calculate_short_range_energy();
            }
            else if (irange == IR::ABINITIO_SHORTMID) {
                const auto sample = samples[i_sample];
                output_energies[i_sample] = calculate_shortmid_range_energy(sample);
            }
            else if (irange == IR::ABINITIO_MID) {
                output_energies[i_sample] = calculate_mid_range_energy();
            }
            else if (irange == IR::MIXED_SHORT) {
                const auto sample = samples[i_sample];
                const auto abinitio_energy = calculate_short_range_energy();
                output_energies[i_sample] = calculate_mixed_range_energy(abinitio_energy, sample);
            }
            else if (irange == IR::MIXED_SHORTMID) {
                const auto sample = samples[i_sample];
                const auto abinitio_energy = calculate_shortmid_range_energy(sample);
                output_energies[i_sample] = calculate_mixed_range_energy(abinitio_energy, sample);
            }
            else if (irange == IR::MIXED_MID) {
                const auto sample = samples[i_sample];
                const auto abinitio_energy = calculate_mid_range_energy();
                output_energies[i_sample] = calculate_mixed_range_energy(abinitio_energy, sample);
            }
            else {
                const auto sample = samples[i_sample];
                output_energies[i_sample] = calculate_long_range_energy(sample);
            }
        }

        return output_energies;
    }

private:
    RescalingModel rescaling_model_;
    InputSampleTransformer transformer_;
    LongRangeEnergyCorrector long_range_corrector_;
    ShortRangeDataPreparer short_range_preparer_;
    ShortRangeEnergyCorrector short_range_corrector_;

    constexpr auto assign_interaction_ranges_(const torch::Tensor& samples) const -> std::vector<IR>
    {
        const long int n_samples = samples.size(0);
        const long int sample_size = samples.size(1);

        auto interaction_ranges = std::vector<IR> {};
        interaction_ranges.reserve(static_cast<std::size_t>(n_samples));

        for (long int i = 0; i < n_samples; ++i) {
            const auto* sample_begin = samples.data_ptr<FP>() + i * sample_size;
            const auto* sample_end = sample_begin + sample_size;

            const auto irange = interact_ranges::classify_interaction_range(sample_begin, sample_end);
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

        // clang-format off
        const auto push_short_sample = [&](const torch::Tensor& sample)
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

            distance_infos.emplace_back(dist_info);
        };

        const auto push_unmodified_sample = [&](const torch::Tensor& sample)
        {
            batch_sidelengths[i_batch++] = torch::from_blob(sample.template data_ptr<FP>(), {6});
        };
        // clang-format on

        for (std::size_t i_sample {}; i_sample < interaction_ranges.size(); ++i_sample) {
            const auto irange = interaction_ranges[i_sample];
            const auto sample = input_sidelengths[static_cast<long int>(i_sample)];

            if (irange == IR::ABINITIO_SHORT || irange == IR::MIXED_SHORT) {
                push_short_sample(sample);
            }
            else if (irange == IR::ABINITIO_SHORTMID || irange == IR::MIXED_SHORTMID) {
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

template <std::floating_point FP, std::size_t NDIM, typename InputSampleTransformer>
class BufferedExtrapolatedPotential
{
public:
    using ExtrapPotential = ExtrapolatedPotential<FP, NDIM, InputSampleTransformer>;

    explicit BufferedExtrapolatedPotential(ExtrapPotential extrap_pot, long int buffer_size)
        : extrap_pot_ {std::move(extrap_pot)}
        , buffer_size_ {buffer_size}
        , number_of_samples_ {0}
        , total_energy_ {FP {0.0}}
    {
        ctr_check_buffer_size_positive_(buffer_size);
        sample_buffer_ = torch::empty({buffer_size, 6});
    }

    constexpr auto add_sample(const coord::FourBodyAttardSideLengths<FP>& side_lengths) -> void
    {
        if (number_of_samples_ == buffer_size_) {
            total_energy_ += evaluate_buffer_(number_of_samples_);
            number_of_samples_ = 0;
        }

        sample_buffer_[number_of_samples_][0] = side_lengths.dist01;
        sample_buffer_[number_of_samples_][1] = side_lengths.dist02;
        sample_buffer_[number_of_samples_][2] = side_lengths.dist03;
        sample_buffer_[number_of_samples_][3] = side_lengths.dist12;
        sample_buffer_[number_of_samples_][4] = side_lengths.dist13;
        sample_buffer_[number_of_samples_][5] = side_lengths.dist23;

        ++number_of_samples_;
    }

    constexpr auto extract_energy() -> FP
    {
        if (number_of_samples_ != 0) {
            total_energy_ += evaluate_buffer_(number_of_samples_);
        }

        const auto energy_to_return = total_energy_;
        total_energy_ = FP {0.0};
        number_of_samples_ = 0;

        return energy_to_return;
    }

private:
    ExtrapPotential extrap_pot_;
    long int buffer_size_;
    long int number_of_samples_;
    FP total_energy_;
    torch::Tensor sample_buffer_;

    constexpr auto ctr_check_buffer_size_positive_(long int buffer_size) const -> void
    {
        if (buffer_size <= 0) {
            throw std::runtime_error("The buffer cannot hold a non-positive number of samples.");
        }
    }

    constexpr auto evaluate_buffer_(long int number_of_samples) const -> FP
    {
        using namespace torch::indexing;

        const auto energies = extrap_pot_.evaluate_batch(sample_buffer_.index({Slice(None, number_of_samples)}));

        return torch::sum(energies).template item<FP>();
    }
};

}  // namespace interact
