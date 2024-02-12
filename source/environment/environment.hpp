#pragma once

#include <concepts>

namespace envir
{

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
