#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <vector>

#include <common/common_utils.hpp>
#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>

namespace coord
{

template <std::floating_point FP>
constexpr auto number_of_box_shifts(FP pair_separation, FP side_length) -> FP
{
    const auto abs_pair_sep = std::fabs(pair_separation);

    const auto shifted_gap = (abs_pair_sep / side_length) - FP {0.5};
    if (shifted_gap > FP {0.0}) {
        const auto sign = common_utils::sign(pair_separation);
        const auto n_shifts = std::ceil(shifted_gap);
        return sign * n_shifts;
    }
    else {
        return FP {0.0};
    }
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto translate_point_near_origin(const Cartesian<FP, NDIM>& point, const BoxSides<FP, NDIM>& box)
    -> Cartesian<FP, NDIM>
{
    std::array<FP, NDIM> coordinates;

    for (std::size_t i {0}; i < NDIM; ++i) {
        const auto n_shifts = number_of_box_shifts(point[i], box[i]);
        coordinates[i] = point[i] - n_shifts * box[i];
    }

    return Cartesian<FP, NDIM> {coordinates};
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto shift_points_together(
    std::size_t i_origin,
    const BoxSides<FP, NDIM>& box,
    const std::vector<Cartesian<FP, NDIM>>& particles
) -> std::vector<Cartesian<FP, NDIM>>
{
    if (particles.size() <= 1) {
        return particles;
    }

    const auto origin = particles[i_origin];

    auto shifted_particles = std::vector<Cartesian<FP, NDIM>> {};
    shifted_particles.reserve(particles.size());

    for (const auto& p : particles) {
        auto shifted_point = translate_point_near_origin(p - origin, box);
        shifted_particles.emplace_back(std::move(shifted_point));
    }

    return shifted_particles;
}

}  // namespace coord
