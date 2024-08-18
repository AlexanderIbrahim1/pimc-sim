#pragma once

#include <iostream>

#include <algorithm>
#include <cmath>
#include <concepts>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include <torch/script.h>

namespace interact
{

namespace rescale
{

template <std::floating_point FP>
struct RescalingLimits
{
    FP from_left;
    FP from_right;
    FP to_left;
    FP to_right;
};

template <std::floating_point FP>
class LinearMap
{
public:
    explicit LinearMap(const RescalingLimits<FP>& rl)
    {
        slope_ = (rl.to_right - rl.to_left) / (rl.from_right - rl.from_left);
        intercept_ = rl.to_left - rl.from_left * slope_;
    }

    constexpr auto operator()(FP x) const -> FP
    {
        return x * slope_ + intercept_;
    }

private:
    FP slope_;
    FP intercept_;
};

template <std::floating_point FP>
constexpr auto invert_rescaling_limits(const RescalingLimits<FP>& rl) -> RescalingLimits<FP>
{
    return RescalingLimits {rl.to_left, rl.to_right, rl.from_left, rl.from_right};
}

template <std::floating_point FP>
class RescalingFunction
{
public:
    explicit RescalingFunction(FP coeff, FP expon, FP dispersion_coeff)
        : coeff_ {coeff}
        , expon_ {expon}
        , dispersion_coeff_ {dispersion_coeff}
    {}

    template <typename Container>
    constexpr auto operator()(const Container& pair_distances) const -> FP
    {
        FP average_pairdist;

        if constexpr (std::is_same_v<Container, torch::Tensor>) {
            average_pairdist = pair_distances.mean().template item<FP>();
        }
        else {
            const auto sum = std::accumulate(pair_distances.cbegin(), pair_distances.cend(), FP {0.0});
            average_pairdist = sum / static_cast<FP>(pair_distances.size());
        }

        const auto expon_contrib = coeff_ * std::exp(-expon_ * average_pairdist);
        // const auto dispersion_contrib = calculate_dispersion_contrib_(average_pairdist);
        const auto denom = static_cast<FP>(std::pow(average_pairdist, 12));
        const auto dispersion_contrib = dispersion_coeff_ / denom;

        return expon_contrib + dispersion_contrib;
    }

private:
    FP coeff_;
    FP expon_;
    FP dispersion_coeff_;

    //     constexpr auto calculate_dispersion_contrib_(FP average_pairdist) const noexcept -> FP
    //     {
    //         const auto pairdist2 = average_pairdist * average_pairdist;
    //         const auto pairdist4 = pairdist2 * pairdist2;
    //         const auto pairdist12 = pairdist4 * pairdist4 * pairdist4;
    //
    //         return dispersion_coeff_ / pairdist12;
    //     }
};

template <std::floating_point FP>
class ForwardEnergyRescaler
{
public:
    explicit ForwardEnergyRescaler(
        const RescalingFunction<FP>& rescaling_function,
        const RescalingLimits<FP>& forward_res_limits
    )
        : rescaling_function_ {rescaling_function}
        , lin_map_ {LinearMap {forward_res_limits}}
    {}

    template <typename Container>
    constexpr auto operator()(FP energy, const Container& pair_distances) const -> FP
    {
        const auto rescale_value = rescaling_function_(pair_distances);

        const auto reduced_energy = energy / rescale_value;
        const auto rescaled_energy = lin_map_(reduced_energy);

        return rescaled_energy;
    }

private:
    RescalingFunction<FP> rescaling_function_;
    LinearMap<FP> lin_map_;
};

template <std::floating_point FP>
class ReverseEnergyRescaler
{
public:
    explicit ReverseEnergyRescaler(
        const RescalingFunction<FP>& rescaling_function,
        const RescalingLimits<FP>& reverse_res_limits
    )
        : rescaling_function_ {rescaling_function}
        , lin_map_ {LinearMap {reverse_res_limits}}
    {}

    template <typename Container>
    constexpr auto operator()(FP rescaled_energy, const Container& pair_distances) const -> FP
    {
        const auto rescale_value = rescaling_function_(pair_distances);

        const auto reduced_energy = lin_map_(rescaled_energy);
        const auto energy = rescale_value * reduced_energy;

        return energy;
    }

private:
    RescalingFunction<FP> rescaling_function_;
    LinearMap<FP> lin_map_;
};

template <std::floating_point FP>
constexpr auto forward_and_reverse_energy_rescalers(
    const RescalingFunction<FP>& rescaling_function,
    const RescalingLimits<FP>& forward_res_limits
) -> std::tuple<ForwardEnergyRescaler<FP>, ReverseEnergyRescaler<FP>>
{
    const auto reverse_res_limits = invert_rescaling_limits(forward_res_limits);

    const auto forward_rescaler = ForwardEnergyRescaler {rescaling_function, forward_res_limits};
    const auto reverse_rescaler = ReverseEnergyRescaler {rescaling_function, reverse_res_limits};

    return {forward_rescaler, reverse_rescaler};
}

template <std::floating_point FP>
constexpr auto forward_rescale_energies(
    const ForwardEnergyRescaler<FP>& forward_rescaler,
    const torch::Tensor& side_length_groups,
    torch::Tensor& energies_to_rescale
) -> void
{
    for (unsigned int i {}; i < energies_to_rescale.size(0); ++i) {
        const auto energy = energies_to_rescale[i].template item<FP>();
        energies_to_rescale[i] = forward_rescaler(energy, side_length_groups[i]);
    }
}

template <std::floating_point FP>
constexpr auto reverse_rescale_energies(
    const ReverseEnergyRescaler<FP>& reverse_rescaler,
    const torch::Tensor& side_length_groups,
    torch::Tensor& energies_to_reverse_rescale
) -> void
{
    for (unsigned int i {}; i < energies_to_reverse_rescale.size(0); ++i) {
        const auto res_energy = energies_to_reverse_rescale[i].template item<FP>();
        energies_to_reverse_rescale[i] = reverse_rescaler(res_energy, side_length_groups[i]);
    }
}

template <std::floating_point FP>
class RescalingEnergyModel
{
public:
    explicit RescalingEnergyModel(
        torch::jit::script::Module&& rescaled_module,
        const ReverseEnergyRescaler<FP>& reverse_rescaler
    )
        : rescaled_module_ {std::move(rescaled_module)}
        , reverse_rescaler_ {reverse_rescaler}
    {}

    constexpr auto operator()(const torch::Tensor& x_data, const torch::Tensor& side_length_groups) const
        -> torch::Tensor
    {
        const long int n_samples = x_data.size(0);

        if (x_data.sizes() != side_length_groups.sizes()) {
            throw std::runtime_error("Received inputs and side length groups of different sizes!");
        }

        if (n_samples == 0) {
            return {};
        }

        const auto rescaled_energies = [&]()
        {
            rescaled_module_.eval();
            torch::NoGradGuard no_grad;
            return rescaled_module_.forward({x_data}).toTensor();
        }();

        for (long int i {}; i < n_samples; ++i) {
            const FP res_energy = rescaled_energies[i].template item<FP>();
            const auto rescaled_energy = reverse_rescaler_(res_energy, side_length_groups[i]);
            rescaled_energies[i] = rescaled_energy;
            // rescaled_energies[i] = reverse_rescaler_(res_energy, side_length_groups[i]);

            // std::cout << "before rescale: energy[" << i << "] = " << static_cast<double>(res_energy) << '\n';
            // std::cout << "after rescale:  energy[" << i << "] = " << static_cast<double>(rescaled_energy) << '\n';
        }

        return rescaled_energies;
    }

private:
    mutable torch::jit::script::Module rescaled_module_;  // .forward() is not const
    ReverseEnergyRescaler<FP> reverse_rescaler_;
};

}  // namespace rescale

}  // namespace interact
