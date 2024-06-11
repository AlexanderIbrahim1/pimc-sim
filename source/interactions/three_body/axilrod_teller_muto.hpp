#pragma once

#include <cmath>
#include <concepts>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <mathtools/mathtools_utils.hpp>

namespace interact
{

template <std::floating_point FP>
class AxilrodTellerMutoPotential
{
public:
    explicit AxilrodTellerMutoPotential(FP c9_coefficient)
        : c9_coefficient_ {c9_coefficient}
    {
        if (c9_coefficient_ < FP {0.0}) {
            auto err_msg = std::stringstream {};
            err_msg << "The c9 coefficient for the AxilrodTellerMuto potential must be positive.\n";
            err_msg << "Found: " << std::scientific << std::setprecision(8) << c9_coefficient_ << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }

    constexpr auto operator()(FP dist01, FP dist02, FP dist12) const noexcept -> FP
    {
        const auto dist01_sq = dist01 * dist01;
        const auto dist02_sq = dist02 * dist02;
        const auto dist12_sq = dist12 * dist12;

        const auto cos1_numer = (dist01_sq + dist02_sq - dist12_sq);
        const auto cos2_numer = (dist01_sq + dist12_sq - dist02_sq);
        const auto cos3_numer = (dist02_sq + dist12_sq - dist01_sq);

        const auto cos_denom = FP {8.0} * dist01_sq * dist12_sq * dist02_sq;
        const auto fterm = FP {3.0} * cos1_numer * cos2_numer * cos3_numer / cos_denom;

        const auto denom = dist01_sq * dist02_sq * dist12_sq + dist01 * dist02 * dist12;

        return c9_coefficient_ * (FP {1.0} + fterm) / denom;
    }

private:
    FP c9_coefficient_;
};

}  // namespace interact
