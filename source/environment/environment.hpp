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

}  // namespace envir
