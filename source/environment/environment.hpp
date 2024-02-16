#pragma once

#include <concepts>

#include <constants/constants.hpp>
#include <constants/conversions.hpp>
#include <environment/envir_utils.hpp>

namespace envir
{

/*
NOTE: for now, we are working with simulations with a single particle species, so the environment
can contains a single value of the thermodynamic lambda. For later uses, we need to expand the scope
to allow for several different lambdas corresponding to several different particle species.
*/

template <std::floating_point FP>
class Environment
{
public:
    constexpr explicit Environment(
        FP thermodynamic_beta,
        FP thermodynamic_tau,
        FP thermodynamic_lambda,
        std::size_t n_particles,
        std::size_t n_timeslices
    )
        : thermodynamic_beta_ {thermodynamic_beta}
        , thermodynamic_tau_ {thermodynamic_tau}
        , thermodynamic_lambda_ {thermodynamic_lambda}
        , n_particles_ {n_particles}
        , n_timeslices_ {n_timeslices}
    {}

    constexpr auto thermodynamic_beta() const noexcept -> FP
    {
        // units of Kelvin^{-1}
        return thermodynamic_beta_;
    }

    constexpr auto thermodynamic_tau() const noexcept -> FP
    {
        // units of Kelvin^{-1}
        return thermodynamic_tau_;
    }

    constexpr auto thermodynamic_lambda() const noexcept -> FP
    {
        // units of Angstroms^2 * Kelvin
        return thermodynamic_lambda_;
    }

    constexpr auto n_particles() const noexcept -> std::size_t
    {
        return n_particles_;
    }

    constexpr auto n_timeslices() const noexcept -> std::size_t
    {
        return n_timeslices_;
    }

private:
    FP thermodynamic_beta_;
    FP thermodynamic_tau_;
    FP thermodynamic_lambda_;
    std::size_t n_timeslices_;
    std::size_t n_particles_;
};

template <std::floating_point FP>
constexpr auto create_finite_temperature_environment(
    FP temperature,
    FP mass_amu,
    std::size_t n_timeslices,
    std::size_t n_particles
) noexcept -> Environment<FP>
{
    const auto thermo_beta = FP {1.0} / temperature;
    const auto thermo_tau = thermo_beta / n_timeslices;
    const auto thermo_lambda = envir_utils::LAMBDA_CONVERSION_FACTOR<FP> / mass_amu;

    return Environment<FP> {thermo_beta, thermo_tau, thermo_lambda, n_particles, n_timeslices};
}

}  // namespace envir
