#pragma once

#include <cmath>
#include <concepts>
#include <stdexcept>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <coordinates/operations.hpp>

namespace impl_interact_dispersion
{

template <std::floating_point FP, std::size_t NDIM>
struct MagnitudeAndDirection
{
    coord::Cartesian<FP, NDIM> direction;
    FP magnitude;
};

template <std::floating_point FP, std::size_t NDIM>
constexpr auto convert_to_magnitude_and_direction(const coord::Cartesian<FP, NDIM>& point)
    -> MagnitudeAndDirection<FP, NDIM>
{
    const auto distance = coord::norm(point);
    const auto unit_vec = point / distance;

    return {unit_vec, distance};
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto quadruplet_contribution(
    const MagnitudeAndDirection<FP, NDIM>& vec_ij,
    const MagnitudeAndDirection<FP, NDIM>& vec_jk,
    const MagnitudeAndDirection<FP, NDIM>& vec_kl,
    const MagnitudeAndDirection<FP, NDIM>& vec_li
) -> FP
{
    const auto prod_of_mags = vec_ij.magnitude * vec_jk.magnitude * vec_kl.magnitude * vec_li.magnitude;
    const auto denominator = prod_of_mags * prod_of_mags * prod_of_mags;

    const auto prod_ijjk = coord::dot_product(vec_ij.direction, vec_jk.direction);
    const auto prod_ijkl = coord::dot_product(vec_ij.direction, vec_kl.direction);
    const auto prod_ijli = coord::dot_product(vec_ij.direction, vec_li.direction);
    const auto prod_jkkl = coord::dot_product(vec_jk.direction, vec_kl.direction);
    const auto prod_jkli = coord::dot_product(vec_jk.direction, vec_li.direction);
    const auto prod_klli = coord::dot_product(vec_kl.direction, vec_li.direction);

    // clang-format off
    auto numerator =
        - FP{1.0}
        + prod_ijjk * prod_ijjk
        + prod_ijkl * prod_ijkl
        + prod_ijli * prod_ijli
        + prod_jkkl * prod_jkkl
        + prod_jkli * prod_jkli
        + prod_klli * prod_klli
        - FP{3.0} * (prod_ijjk * prod_jkkl * prod_ijkl)
        - FP{3.0} * (prod_ijjk * prod_jkli * prod_ijli)
        - FP{3.0} * (prod_ijkl * prod_klli * prod_ijli)
        - FP{3.0} * (prod_jkkl * prod_klli * prod_jkli)
        + FP{9.0} * (prod_ijjk * prod_jkkl * prod_klli * prod_ijli);
    // clang-format on

    return FP {2.0} * numerator / denominator;
}

}  // namespace impl_interact_dispersion

namespace interact
{

namespace disp
{

template <std::floating_point FP, std::size_t NDIM>
class FourBodyDispersionPotential
{
public:
    FourBodyDispersionPotential(FP bade_coefficient)
        : bade_coefficient_ {bade_coefficient}
    {
        m_check_coefficient_is_positive(bade_coefficient_);
    }

    constexpr auto operator()(
        const coord::Cartesian<FP, NDIM>& point0,
        const coord::Cartesian<FP, NDIM>& point1,
        const coord::Cartesian<FP, NDIM>& point2,
        const coord::Cartesian<FP, NDIM>& point3
    ) const -> FP
    {
        const auto vec10 = impl_interact_dispersion::convert_to_magnitude_and_direction(point1 - point0);
        const auto vec20 = impl_interact_dispersion::convert_to_magnitude_and_direction(point2 - point0);
        const auto vec30 = impl_interact_dispersion::convert_to_magnitude_and_direction(point3 - point0);
        const auto vec21 = impl_interact_dispersion::convert_to_magnitude_and_direction(point2 - point1);
        const auto vec31 = impl_interact_dispersion::convert_to_magnitude_and_direction(point3 - point1);
        const auto vec32 = impl_interact_dispersion::convert_to_magnitude_and_direction(point3 - point2);

        // clang-format off
        const auto total_energy =
              impl_interact_dispersion::quadruplet_contribution(vec30, vec32, vec21, vec10)
            + impl_interact_dispersion::quadruplet_contribution(vec20, vec32, vec31, vec10)
            + impl_interact_dispersion::quadruplet_contribution(vec20, vec21, vec31, vec30);
        // clang-format on

        return -bade_coefficient_ * total_energy;
    }

private:
    FP bade_coefficient_;

    void m_check_coefficient_is_positive(FP bade_coefficient) const
    {
        if (bade_coefficient < FP {0.0}) {
            throw std::runtime_error("The bade coefficient must be positive. Found a non-positive value.");
        }
    }
};

}  // namespace disp

}  // namespace interact
