#pragma once

#include <concepts>

#include <constants/constants.hpp>
#include <constants/conversions.hpp>

namespace envir
{

/*
NOTE: for now, we are working with simulations with a single particle species, so the environment
can contains a single value of the thermodynamic lambda. For later uses, we need to expand the scope
to allow for several different lambdas corresponding to several different particle species.

Calculates the thermodynamic lambda of a particle of a given mass (in amu).
The result is in units of Angstroms^2 * Kelvin.
*/
template <std::floating_point FP>
constexpr auto calculate_thermodynamic_lambda(FP mass_amu) noexcept -> FP
{
    const auto hbar = constants::HBAR_IN_JOULES_SECONDS<FP>;
    const auto boltz = constants::BOLTZMANN_CONSTANT_IN_JOULES_PER_KELVIN<FP>;
    const auto kg_per_amu = conversions::KILOGRAMS_PER_AMU<FP>;
    const auto ang_per_m = conversions::ANGSTROMS_PER_METRE<FP>;

    const auto coefficient = hbar * hbar * ang_per_m * ang_per_m / kg_per_amu / boltz;

    return 0.5 * coefficient / mass_amu;
}

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
        return thermodynamic_beta_;
    }

    constexpr auto thermodynamic_tau() const noexcept -> FP
    {
        return thermodynamic_tau_;
    }

    constexpr auto thermodynamic_lambda() const noexcept -> FP
    {
        return thermodynamic_lambda_;
    }

private:
    FP thermodynamic_beta_;
    FP thermodynamic_tau_;
    FP thermodynamic_lambda_;
};

template <std::floating_point FP>
constexpr auto create_finite_temperature_environment(FP temperature, std::size_t n_timeslices) noexcept
    -> Environment<FP>
{
    const auto thermo_beta = FP {1.0} / temperature;
    const auto thermo_tau = thermo_beta / n_timeslices;

    return Environment<FP> {thermo_beta, thermo_tau};
}

}  // namespace envir
