#pragma once

#include <cmath>
#include <concepts>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>

namespace impl_coord
{

template <std::floating_point FP>
struct SixAxisCoordinates
{
    FP coord01;
    FP coord02;
    FP coord03;
    FP coord12;
    FP coord13;
    FP coord23;
};

template <std::floating_point FP>
constexpr auto cartesian_translation(FP x_i, FP x_j, FP box_side, FP center_shift = static_cast<FP>(1.0e-6)) -> FP
{
    auto unrounded_shift = (x_i - x_j) / box_side;

    // NOTE: when encountered with exactly +0.5 or -0.5, `std::round` always rounds to
    //       the nearest even number, i.e. 0; this leads to errors when one of the points
    //       has a coordinate value exactly half of the box length relative to another
    //       point, because the value gets shifted properly when seen from the negative side,
    //       but not from the positive side;
    //
    //       this correction fixes that issue
    const bool too_close_to_half = std::fabs(unrounded_shift - FP {0.5}) > center_shift;
    const auto shift_flag = static_cast<FP>(too_close_to_half);
    unrounded_shift += (shift_flag * center_shift / FP {2.0});

    return box_side * std::round(unrounded_shift);
}

template <std::floating_point FP>
constexpr auto separation_coordinates(FP x0, FP x1, FP x2, FP x3, FP box_side) -> impl_coord::SixAxisCoordinates<FP>
{
    const auto trans01 = cartesian_translation(x0, x1, box_side);
    const auto trans02 = cartesian_translation(x0, x2, box_side);
    const auto trans03 = cartesian_translation(x0, x3, box_side);

    const auto trans12 = trans02 - trans01;
    const auto trans13 = trans03 - trans01;
    const auto trans23 = trans03 - trans02;

    return {
        x0 - x1 - trans01,
        x0 - x2 - trans02,
        x0 - x3 - trans03,
        x1 - x2 - trans12,
        x1 - x3 - trans13,
        x2 - x3 - trans23,
    };
}

}  // namespace impl_coord

namespace coord
{

template <std::floating_point FP>
struct FourBodySideLengths
{
    FP dist01;
    FP dist02;
    FP dist03;
    FP dist12;
    FP dist13;
    FP dist23;
};

template <std::floating_point FP>
constexpr auto FAILED_ATTARD_SIDELENGTHS = FourBodySideLengths<FP> {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0};

enum class EarlyResultType
{
    valid,
    next1,
    next2,
    next3
};

template <std::floating_point FP>
struct EarlyFourBodyAttardResult
{
    EarlyResultType type;
    FourBodySideLengths<FP> sides;
};

template <std::floating_point FP, std::size_t NDIM>
auto four_body_attard_side_lengths_preshift(
    const Cartesian<FP, NDIM>& point0,
    const Cartesian<FP, NDIM>& point1,
    const Cartesian<FP, NDIM>& point2,
    const Cartesian<FP, NDIM>& point3,
    FP cutoff_distance_sq
) -> EarlyFourBodyAttardResult<FP>
{
    const auto dist01_sq = distance_squared(point0, point1);
    if (dist01_sq > cutoff_distance_sq) {
        return {EarlyResultType::next1, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto dist02_sq = distance_squared(point0, point2);
    if (dist02_sq > cutoff_distance_sq) {
        return {EarlyResultType::next2, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto dist03_sq = distance_squared(point0, point3);
    if (dist03_sq > cutoff_distance_sq) {
        return {EarlyResultType::next3, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto dist12_sq = distance_squared(point1, point2);
    if (dist12_sq > cutoff_distance_sq) {
        return {EarlyResultType::next2, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto dist13_sq = distance_squared(point1, point3);
    if (dist13_sq > cutoff_distance_sq) {
        return {EarlyResultType::next3, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto dist23_sq = distance_squared(point2, point3);
    if (dist23_sq > cutoff_distance_sq) {
        return {EarlyResultType::next3, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto dist01 = std::sqrt(dist01_sq);
    const auto dist02 = std::sqrt(dist02_sq);
    const auto dist03 = std::sqrt(dist03_sq);
    const auto dist12 = std::sqrt(dist12_sq);
    const auto dist13 = std::sqrt(dist13_sq);
    const auto dist23 = std::sqrt(dist23_sq);

    return {
        EarlyResultType::valid, {dist01, dist02, dist03, dist12, dist13, dist23}
    };
}

template <std::floating_point FP, std::size_t NDIM>
auto four_body_attard_side_lengths_early(
    const Cartesian<FP, NDIM>& point0,
    const Cartesian<FP, NDIM>& point1,
    const Cartesian<FP, NDIM>& point2,
    const Cartesian<FP, NDIM>& point3,
    const BoxSides<FP, NDIM>& periodic_box,
    FP cutoff_distance_sq
) -> EarlyFourBodyAttardResult<FP>
{
    const auto x_sep = impl_coord::separation_coordinates(point0[0], point1[0], point2[0], point3[0], periodic_box[0]);
    const auto y_sep = impl_coord::separation_coordinates(point0[1], point1[1], point2[1], point3[1], periodic_box[1]);
    const auto z_sep = impl_coord::separation_coordinates(point0[2], point1[2], point2[2], point3[2], periodic_box[2]);

    const auto norm01_sq = norm_squared(x_sep.coord01, y_sep.coord01, z_sep.coord01);
    if (norm01_sq > cutoff_distance_sq) {
        return {EarlyResultType::next1, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto norm02_sq = norm_squared(x_sep.coord02, y_sep.coord02, z_sep.coord02);
    if (norm02_sq > cutoff_distance_sq) {
        return {EarlyResultType::next2, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto norm03_sq = norm_squared(x_sep.coord03, y_sep.coord03, z_sep.coord03);
    if (norm03_sq > cutoff_distance_sq) {
        return {EarlyResultType::next3, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto norm12_sq = norm_squared(x_sep.coord12, y_sep.coord12, z_sep.coord12);
    if (norm12_sq > cutoff_distance_sq) {
        return {EarlyResultType::next2, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto norm13_sq = norm_squared(x_sep.coord13, y_sep.coord13, z_sep.coord13);
    if (norm13_sq > cutoff_distance_sq) {
        return {EarlyResultType::next3, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto norm23_sq = norm_squared(x_sep.coord23, y_sep.coord23, z_sep.coord23);
    if (norm23_sq > cutoff_distance_sq) {
        return {EarlyResultType::next3, FAILED_ATTARD_SIDELENGTHS<FP>};
    }

    const auto norm01 = std::sqrt(norm01_sq);
    const auto norm02 = std::sqrt(norm02_sq);
    const auto norm03 = std::sqrt(norm03_sq);
    const auto norm12 = std::sqrt(norm12_sq);
    const auto norm13 = std::sqrt(norm13_sq);
    const auto norm23 = std::sqrt(norm23_sq);

    return {
        EarlyResultType::valid, {norm01, norm02, norm03, norm12, norm13, norm23}
    };
}

}  // namespace coord
