#pragma once

#include <concepts>

namespace envir
{

/*
NOTE: for now, we are working with simulations with a single particle species, so the environment
can contains a single value of the thermodynamic lambda. For later uses, we need to expand the scope
to allow for several different lambdas corresponding to several different particle species.

TODO: move the constants into a separate header file.
TODO: to avoid numerical instability, precompute the constants needed to calculate thermodynamic lambda
*/
template <std::floating_point FP>
constexpr auto calculate_thermodynamic_lambda(FP mass_amu) noexcept -> FP
{
    const auto hbar = FP {1.054'571'817};
}

template <std::floating_point FP>
class Environment
{
public:
    constexpr explicit Environment(FP thermodynamic_beta, FP thermodynamic_tau)
        : thermodynamic_beta_ {thermodynamic_beta}
        , thermodynamic_tau_ {thermodynamic_tau}
    {}

    constexpr auto thermodynamic_beta() const -> FP
    {
        return thermodynamic_beta_;
    }

    constexpr auto thermodynamic_tau() const -> FP
    {
        return thermodynamic_tau_;
    }

private:
    FP thermodynamic_beta_;
    FP thermodynamic_tau_;
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
