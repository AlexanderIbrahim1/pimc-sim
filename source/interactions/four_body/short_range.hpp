#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <stdexcept>

#include <common/common_utils.hpp>

namespace interact
{

namespace short_range
{

template <std::floating_point FP>
struct ExtrapolationEnergies
{
    FP lower;
    FP upper;
};

template <std::floating_point FP>
struct ExtrapolationSideLengths
{
    std::array<FP, 6> lower;
    std::array<FP, 6> upper;
};

template <std::floating_point FP>
struct ExtrapolationDistanceInfo
{
    FP r_short_range;
    FP r_lower;
    FP r_upper;
};

template <typename T>
struct CachedValue
{
    T value;
    bool is_cached;
};

template <std::floating_point FP>
class LinearEnergyExtrapolator
{
public:
    explicit LinearEnergyExtrapolator(ExtrapolationEnergies<FP> energies, ExtrapolationDistanceInfo<FP> distances)
        : energies_ {energies}
        , distances_ {distances}
        , cached_slope_ {0.0, false}
        , cached_energy_ {0.0, false}
    {}

    constexpr auto slope() -> FP
    {
        if (cached_slope_.is_cached) {
            return cached_slope_.value;
        }
        else {
            const auto slope_ = (energies_.upper - energies_.lower) / (distances_.r_upper - distances_.r_lower);
            cached_slope_ = {slope_, true};
            return slope_;
        }
    }

    constexpr auto energy() -> FP
    {
        if (cached_energy_.is_cached) {
            return cached_energy_.value;
        }
        else {
            const auto dist_shift = distances_.r_short_range - distances_.r_lower;
            const auto energy_ = energies_.lower + slope() * dist_shift;

            cached_energy_ = {energy_, true};
            return energy_;
        }
    }

private:
    ExtrapolationEnergies<FP> energies_;
    ExtrapolationDistanceInfo<FP> distances_;
    CachedValue<FP> cached_slope_;
    CachedValue<FP> cached_energy_;
};

template <std::floating_point FP>
class ExponentialEnergyExtrapolator
{
public:
    explicit ExponentialEnergyExtrapolator(
        ExtrapolationEnergies<FP> energies,
        ExtrapolationDistanceInfo<FP> distances,
        FP abs_energy_floor = static_cast<FP>(1.0e-8)
    )
        : energies_ {energies}
        , distances_ {distances}
        , abs_energy_floor_ {abs_energy_floor}
        , cached_slope_ {0.0, false}
        , cached_energy_ {0.0, false}
    {
        if (abs_energy_floor <= 0.0) {
            throw std::runtime_error("The absolute energy floor value must be a small positive number.");
        }
    }

    constexpr auto slope() -> FP
    {
        if (cached_slope_.is_cached) {
            return cached_slope_.value;
        }
        else {
            const auto lower_floor = std::max(abs_energy_floor_, std::fabs(energies_.lower));
            const auto upper_floor = std::max(abs_energy_floor_, std::fabs(energies_.upper));

            const auto log_energy_sep = std::log(upper_floor / lower_floor);
            const auto log_distance_sep = distances_.r_upper - distances_.r_lower;

            const auto slope_ = -log_energy_sep / log_distance_sep;

            cached_slope_ = {slope_, true};

            return slope_;
        }
    }

    constexpr auto energy() -> FP
    {
        if (cached_energy_.is_cached) {
            return cached_energy_.value;
        }
        else {
            const auto dist_shift = distances_.r_short_range - distances_.r_lower;
            const auto energy_ = energies_.lower * std::exp(-slope() * dist_shift);

            cached_energy_ = {energy_, true};

            return energy_;
        }
    }

    constexpr auto is_magnitude_increasing_with_distance() -> bool
    {
        return slope() < 0.0;
    }

private:
    ExtrapolationEnergies<FP> energies_;
    ExtrapolationDistanceInfo<FP> distances_;
    FP abs_energy_floor_;
    CachedValue<FP> cached_slope_;
    CachedValue<FP> cached_energy_;
};

template <std::floating_point FP>
class ShortRangeDataPreparer
{
public:
    explicit ShortRangeDataPreparer(FP scaling_step, FP short_range_cutoff)
    {
        ctr_check_scaling_step_is_positive(scaling_step);
        ctr_check_short_range_cutoff_is_positive(short_range_cutoff);

        side_lower_ = short_range_cutoff;
        side_upper_ = short_range_cutoff + scaling_step;
    }

    // NOTE: the `prepare` function requires (but doesn't check) that
    //       0.0 < shortest_side <= m_short_range_cutoff
    constexpr auto prepare(const FP* begin, const FP* end) const
    {
        const auto side_shortest = *(std::min_element(begin, end));
        const auto scaling_ratio_lower = side_lower_ / side_shortest;
        const auto scaling_ratio_upper = side_upper_ / side_shortest;

        const auto sample_lower = scaled_sample_(scaling_ratio_lower, begin, end);
        const auto sample_upper = scaled_sample_(scaling_ratio_upper, begin, end);

        const auto extrap_dist_info = ExtrapolationDistanceInfo {side_shortest, side_lower_, side_upper_};
        const auto extrap_sample = ExtrapolationSideLengths {sample_lower, sample_upper};

        return std::make_tuple(extrap_sample, extrap_dist_info);
    }

private:
    FP side_lower_;
    FP side_upper_;

    constexpr auto ctr_check_scaling_step_is_positive(FP scaling_step) const
    {
        if (scaling_step <= FP {0.0}) {
            throw std::runtime_error("The scaling step for the short-range extrapolation must be positive.");
        }
    }

    constexpr auto ctr_check_short_range_cutoff_is_positive(FP short_range_cutoff) const
    {
        if (short_range_cutoff <= FP {0.0}) {
            throw std::runtime_error("The short-range cutoff must be positive.");
        }
    }

    constexpr auto scaled_sample_(FP scaling, const FP* begin, const FP* end) const -> std::array<FP, 6>
    {
        std::array<FP, 6> scaled_sample;
        std::size_t i = 0;
        for (auto it = begin; it < end; ++it) {
            const auto sample_value = *it;
            scaled_sample[i++] = scaling * sample_value;
        }

        return scaled_sample;
    }
};

template <std::floating_point FP>
class ShortRangeEnergyCorrector
{
public:
    explicit ShortRangeEnergyCorrector(FP slope_min, FP slope_max)
    {
        ctr_check_slope_is_positive(slope_min);
        ctr_check_slope_is_positive(slope_max);
        ctr_check_slope_order(slope_min, slope_max);

        slope_min_ = slope_min;
        slope_max_ = slope_max;
    }

    constexpr auto operator()(
        const ExtrapolationEnergies<FP>& extrap_energies,
        const ExtrapolationDistanceInfo<FP>& extrap_dist_info
    ) const -> FP
    {
        auto linear_extrapolator = LinearEnergyExtrapolator(extrap_energies, extrap_dist_info);

        if (!common::is_same_sign(extrap_energies.lower, extrap_energies.upper)) {
            return linear_extrapolator.energy();
        }

        auto expon_extrapolator = ExponentialEnergyExtrapolator(extrap_energies, extrap_dist_info);

        if (expon_extrapolator.is_magnitude_increasing_with_distance()) {
            return linear_extrapolator.energy();
        }

        if (expon_extrapolator.slope() <= slope_min_) {
            return expon_extrapolator.energy();
        }
        else if (expon_extrapolator.slope() >= slope_max_) {
            return linear_extrapolator.energy();
        }
        else {
            const auto frac_linear = common::smooth_01_transition(expon_extrapolator.slope(), slope_min_, slope_max_);
            const auto frac_expon = FP {1.0} - frac_linear;

            const auto energy_linear = linear_extrapolator.energy();
            const auto energy_expon = expon_extrapolator.energy();

            return frac_linear * energy_linear + frac_expon * energy_expon;
        }
    }

private:
    FP slope_min_;
    FP slope_max_;

    constexpr void ctr_check_slope_is_positive(FP slope) const
    {
        if (slope <= FP {0.0}) {
            throw std::runtime_error("The slope bounds for the short range extrpolation must be positive.");
        }
    }

    constexpr void ctr_check_slope_order(FP slope_min, FP slope_max) const
    {
        if (slope_min >= slope_max) {
            throw std::runtime_error("The maximum slope must be greater than the minimum slope.");
        }
    }
};

}  // namespace short_range

}  // namespace interact
