#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <utility>

#include <interactions/three_body/axilrod_teller_muto.hpp>
#include <mathtools/interpolate/trilinear_interp.hpp>

namespace interact
{

template <std::floating_point FP>
struct JacobiPoint
{
    FP r;
    FP s;
    FP cosu;
};

template <std::floating_point FP>
auto ordered_pairdistances(FP r_ab, FP r_ac, FP r_bc) -> std::array<FP, 3>
{
    auto distances = std::array<FP, 3> {r_ab, r_ac, r_bc};
    std::sort(distances.begin(), distances.end());

    return distances;
}

template <std::floating_point FP>
auto jacobi_from_pair_distances_ordered(FP r01, FP r02, FP r12) -> JacobiPoint<FP>
{
    const auto r01_sq = r01 * r01;
    const auto r02_sq = r02 * r02;
    const auto r12_sq = r12 * r12;

    const auto s_unsc = std::sqrt(FP {0.5} * (r02_sq + r12_sq - FP {0.5} * r01_sq));
    const auto cosu_unclamped = (r12_sq - r02_sq) / (FP {2.0} * r01 * s_unsc);

    // floating-point errors sometimes causes `cosu_unclamped` to be very slightly
    // greater than 1.0 or less than 0.0 (like -0.00000000001 or something), and
    // this causes issues later for the grid interpolation
    const auto cosu = std::clamp(cosu_unclamped, FP {0.0}, FP {1.0});
    const auto s_min = FP {0.5} * r01 * (cosu + std::sqrt(FP {3.0} + cosu * cosu));

    // floating-point errors sometimes causes `s_unsc / s_min` to be very slightly
    // lower than 1.0, which causes grid interpolation issues
    const auto s = std::max(s_unsc / s_min, FP {1.0});

    return {r01, s, cosu};
}

template <std::floating_point FP>
auto jacobi_from_pair_distances_unordered(FP r_ab, FP r_ac, FP r_bc) -> JacobiPoint<FP>
{
    const auto [r01, r02, r12] = ordered_pairdistances(r_ab, r_ac, r_bc);
    return jacobi_from_pair_distances_ordered(r01, r02, r12);
}

/*
The isotropic three-body potential energy surface for parahydrogen
published in "J. Chem. Phys. 156, 044301 (2022)"
*/
template <std::floating_point FP>
class ThreeBodyParaH2Potential
{
public:
    ThreeBodyParaH2Potential(mathtools::TrilinearInterpolator<FP> interpolator, FP c9_coefficient)
        : interpolator_ {std::move(interpolator)}
        , atm_potential_ {c9_coefficient}
        , r_min_ {interpolator_.get_limits0().lower()}
        , r_max_ {interpolator_.get_limits0().upper()}
        , s_max_ {interpolator_.get_limits1().upper()}
    {}

    auto operator()(FP dist01, FP dist02, FP dist12) const noexcept -> FP
    {
        const auto [r_, s, cosu] = jacobi_from_pair_distances_unordered(dist01, dist02, dist12);
        const auto r = std::max(r_, r_min_);

        if (r < r_max_ && s < s_max_) {
            return interpolator_(r, s, cosu);
        }
        else {
            return atm_potential_(dist01, dist02, dist12);
        }
    }

private:
    mathtools::TrilinearInterpolator<FP> interpolator_;
    AxilrodTellerMutoPotential<FP> atm_potential_;
    FP r_min_;
    FP r_max_;
    FP s_max_;
};

template <std::floating_point FP>
struct EarlyRejectInfo
{
    FP r_shortest_lower_limit;
    FP r_upper_limit;
};

template <std::floating_point FP>
class EarlyRejectorThreeBodyParaH2Potential
{
public:
    EarlyRejectorThreeBodyParaH2Potential(mathtools::TrilinearInterpolator<FP> interpolator, EarlyRejectInfo<FP> info)
        : interpolator_ {std::move(interpolator)}
        , info_ {info}
        , r_max_ {interpolator_.get_limits0().upper()}
        , s_max_ {interpolator_.get_limits1().upper()}
    {}

    auto operator()(FP dist01, FP dist02, FP dist12) const noexcept -> FP
    {
        // reject if any pair distances are too long
        if (dist01 > info_.r_upper_limit || dist02 > info_.r_upper_limit || dist12 > info_.r_upper_limit) {
            return FP {0.0};
        }

        const auto [r01, r02, r12] = ordered_pairdistances(dist01, dist02, dist12);

        // reject if the shortest pair distance is too long (with a separate rejection criterion)
        if (r01 > info_.r_shortest_lower_limit) {
            return FP {0.0};
        }

        const auto [r, s, cosu] = jacobi_from_pair_distances_ordered(r01, r02, r12);

        // reject if the point is outside the grid; note that depending on the values in EarlyRejectInfo,
        // any point that would have ended up outside the grid would have been rejected already
        if (r > r_max_ || s > s_max_) {
            return FP {0.0};
        }

        return interpolator_(r, s, cosu);
    }

private:
    mathtools::TrilinearInterpolator<FP> interpolator_;
    EarlyRejectInfo<FP> info_;
    FP r_max_;
    FP s_max_;
};

}  // namespace interact
