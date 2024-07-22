#pragma once

#include <cmath>
#include <concepts>
#include <tuple>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>

namespace coord
{

template <std::floating_point FP, std::size_t NDIM>
constexpr auto dot_product(const Cartesian<FP, NDIM>& point0, const Cartesian<FP, NDIM>& point1) noexcept -> FP
{
    auto result = FP {};

    for (std::size_t i_dim {}; i_dim < NDIM; ++i_dim) {
        const auto term = point0[i_dim] * point1[i_dim];
        result += term;
    }

    return result;
}

template <std::floating_point FP>
constexpr auto six_side_lengths_to_cartesian(FP r01, FP r02, FP r03, FP r12, FP r13, FP r23, FP tolerance)
{
    /*
        NOTE
        Sometimes, floating-point errors cause the arguments of `std::sqrt` for the calculations of `y2`
        and `z3` to be very slightly negative.

        This function assumes that the six side lengths passed as arguments are always capable of forming
        a real four-body geometry, and that each time `y2` and `z3` receive a negative argument, it is a
        borderline case where the argument is just very slightly negative.

        It will not check if the argument is "very negative" (an indication that the six side lengths do
        not actually form a proper four-body geometry).
    */

    const auto r01_sq = r01 * r01;
    const auto r02_sq = r02 * r02;
    const auto r03_sq = r03 * r03;
    const auto r12_sq = r12 * r12;
    const auto r13_sq = r13 * r13;
    const auto r23_sq = r23 * r23;

    // calculate x2, x3
    const auto x1 = r01;
    const auto x2 = (r01_sq + r02_sq - r12_sq) / (FP {2.0} * r01);
    const auto x3 = (r03_sq - r13_sq + r01_sq) / (FP {2.0} * r01);

    // calculate y2, y3
    /*
        NOTE
        The way the points are constructed, we set point0 at the origin, and point1 somewhere on the x-axis.

        In the general case, point2 is in the x-y plane, but not on the x-axis. As a result, point3 has a
        definite, unique location in space, and the calculation proceeds properly.

        However, when y2 == 0.0, point2 lies on the x-axis. In this special case, one of the constraints on
        the location of point3 is removed. We can construct a ring of radius sqrt(r03^2 - x3^2) that lies
        parallel to the y-z plane and is centred at x3 on the x-axis.

        Thus point3 can lie anywhere on this ring, and its relative pair distances from point0, point1, and
        point2 will remain unchanged. So it doesn't matter where on the ring we place it. For simplicity, we
        can just set y3 == 0.0 as well (making both y2 and y3 zero in this case).

        Without taking care of this corner case, y3 blows up and the calculations don't work properly.

        To make sure that y3 == 0.0, I also performed some runs where I set y3 using:
        '''
            const FP s = std::sqrt(r03_sq - x3 * x3);
            const FP y3_ = s * static_cast<FP>(std::sin(1.23456));
        '''
        where the argument to the sin function was set to lots of different numbers; they all gave the same
        result!

    */
    const auto y1 = FP {0.0};
    const auto [y2, y3] = [&]()
    {
        const auto y2_inner = r02_sq - x2 * x2;
        const bool y2_large_enough = y2_inner > tolerance;

        if (y2_large_enough) {
            const auto y2_ = std::sqrt(y2_inner);
            const auto y3_ = (r03_sq - r23_sq + r02_sq - FP {2.0} * x2 * x3) / (FP {2.0} * y2_);
            return std::make_tuple(y2_, y3_);
        }
        else {
            return std::make_tuple(FP {0.0}, FP {0.0});
        }
    }();

    // calculate z2, z3
    const auto z1 = FP {0.0};
    const auto z2 = FP {0.0};
    const auto z3 = [&]()
    {
        const auto z3_inner = r03_sq - x3 * x3 - y3 * y3;
        const auto z3_large_enough = static_cast<FP>(z3_inner > FP {0.0});
        return std::sqrt(z3_large_enough * z3_inner);
    }();

    return std::make_tuple(
        Cartesian<FP, NDIM> {0.0, 0.0, 0.0},
        Cartesian<FP, NDIM> {x1, y1, z1},
        Cartesian<FP, NDIM> {x2, y2, z2},
        Cartesian<FP, NDIM> {x3, y3, z3}
    );
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto cartesian_to_six_side_lengths(
    const Cartesian<FP, NDIM>& point0,
    const Cartesian<FP, NDIM>& point1,
    const Cartesian<FP, NDIM>& point2,
    const Cartesian<FP, NDIM>& point3
)
{
    const auto r01 = coord::distance(point0, point1);
    const auto r02 = coord::distance(point0, point2);
    const auto r03 = coord::distance(point0, point3);
    const auto r12 = coord::distance(point1, point2);
    const auto r13 = coord::distance(point1, point3);
    const auto r23 = coord::distance(point2, point3);

    return std::make_tuple(r01, r02, r03, r12, r13, r23);
}

}  // namespace coord
