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
    constexpr explicit Environment(FP thermodynamic_beta, FP thermodynamic_tau, FP thermodynamic_lambda)
        : thermodynamic_beta_ {thermodynamic_beta}
        , thermodynamic_tau_ {thermodynamic_tau}
        , thermodynamic_lambda_ {thermodynamic_lambda}
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

private:
    FP thermodynamic_beta_;
    FP thermodynamic_tau_;
    FP thermodynamic_lambda_;
};

template <std::floating_point FP>
constexpr auto create_finite_temperature_environment(FP temperature, std::size_t n_timeslices, FP mass_amu) noexcept
    -> Environment<FP>
{
    const auto thermo_beta = FP {1.0} / temperature;
    const auto thermo_tau = thermo_beta / n_timeslices;
    const auto thermo_lambda = envir_utils::LAMBDA_CONVERSION_FACTOR<FP> / mass_amu;

    return Environment<FP> {thermo_beta, thermo_tau, thermo_lambda};
}

}  // namespace envir
