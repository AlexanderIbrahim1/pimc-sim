#pragma once

#include <concepts>
#include <utility>
#include <vector>

#include <mathtools/interpolate/linear_interp.hpp>

namespace interact
{

enum class LongRangeCheckStatus
{
    ON,
    OFF
};

template <std::floating_point FP, LongRangeCheckStatus Status>
class FSHPairPotential
{
public:
    explicit FSHPairPotential(std::vector<FP> energies, FP r2_min, FP r2_max)
        : interpolator_ {energies, r2_min, r2_max}
        , c6_multipole_coeff_ {ctr_calculate_c6_multipole_coeff(energies, r2_min, r2_max)}
        , r2_max_ {r2_max}
    {}

    constexpr auto operator()(FP dist_squared) const noexcept -> FP
    {
        if constexpr (Status == LongRangeCheckStatus::OFF) {
            return interpolator_(dist_squared);
        }
        else {
            if (dist_squared >= r2_max_) {
                const auto dist_pow6 = dist_squared * dist_squared * dist_squared;
                return c6_multipole_coeff_ / (dist_pow6);
            }
            else {
                return interpolator_(dist_squared);
            }
        }
    }

private:
    interp::RegularLinearInterpolator<FP> interpolator_;
    FP c6_multipole_coeff_;
    FP r2_max_;

    constexpr auto ctr_calculate_c6_multipole_coeff(const std::vector<FP>& energies, FP r2_min, FP r2_max) -> FP
    {
        // NOTES:
        // - not too many useful names for the temporary variables created in this function
        // - the interpolator will have thrown an exception if size < 2, so vector accesses should be okay
        const auto size = energies.size();
        const auto r2_step_size = (r2_max - r2_min) / static_cast<FP>(size - 1);

        const auto energy_step = energies[size - 1] - energies[size - 2];

        const auto r2_last = r2_max;
        const auto r2_sec_last = r2_max - r2_step_size;

        const auto r2_term0 = r2_sec_last * r2_sec_last * r2_sec_last;
        const auto r2_term1 = r2_last * r2_last * r2_last;

        return energy_step / (FP {1.0} / r2_term0 - FP {1.0} / r2_term1);
    }
};

}  // namespace interact
