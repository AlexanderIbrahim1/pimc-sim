#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <utility>

#include <mathtools/interpolate/trilinear_interp.hpp>
#include <interactions/three_body/three_body_pointwise.hpp>

namespace interact
{

template <std::floating_point FP>
struct JacobiPoint
{
    FP r;
    FP s;
    FP cosu;
};

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
    {}

    auto operator()(FP dist01, FP dist02, FP dist12) const noexcept -> FP
    {
        const auto [r_, s, cosu] = jacobi_from_pair_distances_unordered_(dist01, dist02, dist12);

        const auto r_min = interpolator_.get_limits0().lower();
        const auto r_max = interpolator_.get_limits0().upper();
        const auto s_max = interpolator_.get_limits1().upper();
        const auto r = std::max(r_, r_min);

        if (r > r_max || s > s_max) {
            return atm_potential_(dist01, dist02, dist12);
        } else {
            return interpolator_(r, s, cosu);
        }
    }

private:
    mathtools::TrilinearInterpolator<FP> interpolator_;
    AxilrodTellerMuto<FP> atm_potential_;

    auto jacobi_from_pair_distances_(FP r01, FP r02, FP r12) -> JacobiPoint<FP>
    {
        const auto r01_sq = r01 * r01;
        const auto r02_sq = r02 * r02;
        const auto r12_sq = r12 * r12;

        const auto r = std::sqrt(r01);
        const auto s_unsc = std::sqrt( FP{0.5} * (r02_sq + r12_sq - FP{0.5} * r01_sq));
        const auto cosu_unclamped = (r02_sq - r12_sq) / (FP{2.0} * r * s_unsc);

        // floating-point errors sometimes causes `cosu_unclamped` to be very slightly
        // greater than 1.0 or less than 0.0 (like -0.00000000001 or something), and
        // this causes issues later for the grid interpolation
        const auto cosu = std::clamp(cosu_unclamped, FP{0.0}, FP{1.0});

        const auto s_min = FP{0.5} * r * (cosu + std::sqrt(FP{3.0} + cosu * cosu));

        // floating-point errors sometimes causes `s_unsc / s_min` to be very slightly
        // lower than 1.0, which causes grid interpolation issues
        const auto s = std::max(s_unsc / s_min, FP{1.0});

        return {r, s, cosu};
    }

    auto ordered_pairdistances_(FP r_ab, FP r_ac, FP r_bc) -> std::array<FP, 3>
    {
        auto distances = std::array<FP, 3> {r_ab, r_ac, r_bc};
        std::sort(distances.begin(), distances.end());

        return distances;
    }

    auto jacobi_from_pair_distances_unordered_(FP r_ab, FP r_ac, FP r_bc) -> JacobiPoint<FP>
    {
        const auto [r01, r02, r12] = ordered_pairdistances_(r_ab, r_ac, r_bc);
        return jacobi_from_pair_distances_(r01, r02, r12);
    }
};


template <std::floating_point FP>
class ThreeBodyParaH2PotentialEarlyRejector : public ThreeBodyParaH2Potential<FP>
{
    using ThreeBodyParaH2Potential<FP>::ThreeBodyParaH2Potential;

    auto interaction_with_early_rejection(FP dist01, FP dist02, FP dist12, FP dist_cutoff) const noexcept -> FP {
        if (dist01 > dist_cutoff || dist02 > dist_cutoff || dist12 > dist_cutoff) {
            return FP{0.0};
        }

        const auto [r01, r02, r12] = ordered_pairdistances_(dist01, dist02, dist12);
    }
};



}  // namespace interact
