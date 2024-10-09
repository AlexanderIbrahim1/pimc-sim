#pragma once

#include <concepts>
#include <tuple>
#include <type_traits>

#include <torch/script.h>

#include <common/common_utils.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/operations.hpp>

#include <interactions/four_body/dispersion_potential.hpp>

namespace interact
{

namespace long_range
{

template <std::floating_point FP, std::size_t NDIM>
class LongRangeEnergyCorrector
{
public:
    using Point3D = typename coord::Cartesian<FP, NDIM>;
    using DispersionPot = typename disp::FourBodyDispersionPotential<FP, NDIM>;

    explicit LongRangeEnergyCorrector(
        DispersionPot dispersion_potential,
        FP long_range_cutoff_begin,
        FP long_range_cutoff_end
    )
        : dispersion_potential_ {dispersion_potential}
        , long_range_cutoff_begin_ {long_range_cutoff_begin}
        , long_range_cutoff_end_ {long_range_cutoff_end}
    {}

    constexpr auto dispersion(const Point3D& p0, const Point3D& p1, const Point3D& p2, const Point3D& p3) const -> FP
    {
        return dispersion_potential_(p0, p1, p2, p3);
    }

    template <typename Container>
    constexpr auto dispersion(const Container& pair_distances) const -> FP
    {
        const auto& [r01, r02, r03, r12, r13, r23] = unpack_six_side_lengths_<Container>(pair_distances);
        const auto& [p0, p1, p2, p3] = coord::six_side_lengths_to_cartesian<FP, NDIM>(r01, r02, r03, r12, r13, r23);
        return dispersion(p0, p1, p2, p3);
    }

    template <typename Container>
    constexpr auto mixed(FP abinitio_energy, const Container& pair_distances) const -> FP
    {
        const auto& [r01, r02, r03, r12, r13, r23] = unpack_six_side_lengths_<Container>(pair_distances);
        const auto& [p0, p1, p2, p3] = coord::six_side_lengths_to_cartesian<FP, NDIM>(r01, r02, r03, r12, r13, r23);
        const auto average_sidelength = common::calculate_mean(r01, r02, r03, r12, r13, r23);

        return mixed_energy_(p0, p1, p2, p3, abinitio_energy, average_sidelength);
    }

    constexpr auto mixed(FP abinitio_energy, const Point3D& p0, const Point3D& p1, const Point3D& p2, const Point3D& p3)
        const -> FP
    {
        const auto& [r01, r02, r03, r12, r13, r23] = coord::cartesian_to_six_side_lengths(p0, p1, p2, p3);
        const auto average_sidelength = common::calculate_mean(r01, r02, r03, r12, r13, r23);

        return mixed_energy_(p0, p1, p2, p3, abinitio_energy, average_sidelength);
    }

private:
    DispersionPot dispersion_potential_;
    FP long_range_cutoff_begin_;
    FP long_range_cutoff_end_;

    constexpr auto mixed_energy_(
        const Point3D& point0,
        const Point3D& point1,
        const Point3D& point2,
        const Point3D& point3,
        FP abinitio_energy,
        FP average_sidelength
    ) const -> FP
    {
        const auto dispersion_energy = dispersion_potential_(point0, point1, point2, point3);
        const auto frac_dispersion = frac_dispersion_(average_sidelength);
        const auto frac_abinitio = FP {1.0} - frac_dispersion;

        const auto mixed_energy = frac_dispersion * dispersion_energy + frac_abinitio * abinitio_energy;

        return mixed_energy;
    }

    constexpr auto frac_dispersion_(FP average_sidelength) const -> FP
    {
        return common::smooth_01_transition(average_sidelength, long_range_cutoff_begin_, long_range_cutoff_end_);
    }

    template <typename Container>
    constexpr auto unpack_six_side_lengths_(const Container& pair_distances) const
    {
        if constexpr (std::is_same_v<Container, torch::Tensor>) {
            const auto r01 = pair_distances[0].template item<FP>();
            const auto r02 = pair_distances[1].template item<FP>();
            const auto r03 = pair_distances[2].template item<FP>();
            const auto r12 = pair_distances[3].template item<FP>();
            const auto r13 = pair_distances[4].template item<FP>();
            const auto r23 = pair_distances[5].template item<FP>();

            return std::make_tuple(r01, r02, r03, r12, r13, r23);
        }
        else {
            return pair_distances;
        }
    }
};

}  // namespace long_range

}  // namespace interact
