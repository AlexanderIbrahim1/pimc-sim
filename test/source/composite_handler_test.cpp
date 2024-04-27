#include <cmath>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "interactions/handlers/composite_interaction_handler.hpp"
#include "interactions/handlers/full_interaction_handler.hpp"
#include "interactions/three_body/three_body_pointwise.hpp"
#include "interactions/three_body/three_body_pointwise_wrapper.hpp"
#include "interactions/two_body/two_body_pointwise.hpp"
#include "interactions/two_body/two_body_pointwise_wrapper.hpp"
#include "worldline/worldline.hpp"

using Point2D = coord::Cartesian<double, 2>;
using Point3D = coord::Cartesian<float, 3>;

constexpr auto equilateral_triangle_points(double side_length) -> std::vector<Point2D>
{
    const auto half_side = 0.5 * side_length;
    const auto height = std::sqrt(3.0 / 4.0) * side_length;

    return {
        Point2D {-half_side, 0.0   },
         Point2D {half_side,  0.0   },
         Point2D {0.0,        height}
    };
}

constexpr auto square_points(float side) -> std::vector<Point3D>
{
    return {
        Point3D {0.0f, 0.0f, 0.0f},
         Point3D {side, 0.0f, 0.0f},
         Point3D {0.0f, side, 0.0f},
         Point3D {side, side, 0.0f}
    };
}

TEST_CASE("test composite full handler : equilateral triangle", "CompositeFullInteractionHandler")
{
    const auto side_length = 1.0;
    const auto points = equilateral_triangle_points(side_length);

    // worldline of 1 timeslice with 3 points
    const auto worldlines = worldline::worldlines_from_positions<double, 2>(points, 1);

    // the FullPairInteractionHandler takes as an argument a functor that takes
    // two points and returns a floating-point value as a result
    auto pairpot = interact::LennardJonesPotential {1.0, 1.0};
    auto pairpot_wrapper = interact::PairDistancePotential<decltype(pairpot), double, 2> {pairpot};
    auto pair_interaction_handler =
        interact::FullPairInteractionHandler<decltype(pairpot_wrapper), double, 2> {pairpot_wrapper};

    // the FullTripletInteractionHandler takes as an argument a functor that takes
    // three points and returns a floating-point value as a result
    auto tripletpot = interact::AxilrodTellerMuto {1.0};
    auto tripletpot_wrapper = interact::TripletDistancePotential<decltype(tripletpot), double, 2> {tripletpot};
    auto triplet_interaction_handler =
        interact::FullTripletInteractionHandler<decltype(tripletpot_wrapper), double, 2> {tripletpot_wrapper};

    using PairType = decltype(pair_interaction_handler);
    using TripletType = decltype(triplet_interaction_handler);

    const auto interaction_handler_2b3b = interact::CompositeFullInteractionHandler<double, 2, PairType, TripletType> {
        pair_interaction_handler, triplet_interaction_handler};

    const auto actual = interaction_handler_2b3b(0, worldlines[0]);
    const auto expected_direct = 3.0 * pairpot(side_length) + tripletpot(side_length, side_length, side_length);
    const auto expected_sep_handlers =
        pair_interaction_handler(0, worldlines[0]) + triplet_interaction_handler(0, worldlines[0]);

    REQUIRE_THAT(actual, Catch::Matchers::WithinRel(expected_direct));
    REQUIRE_THAT(actual, Catch::Matchers::WithinRel(expected_sep_handlers));
}

TEST_CASE("test composite full handler : square", "CompositeFullInteractionHandler")
{
    const auto side_length = 1.0f;
    const auto points = square_points(side_length);

    // worldline of 1 timeslice with 4 points
    const auto worldlines = worldline::worldlines_from_positions<float, 3>(points, 1);

    // the FullPairInteractionHandler takes as an argument a functor that takes
    // two points and returns a floating-point value as a result
    auto pairpot = interact::LennardJonesPotential {1.0f, 1.0f};
    auto pairpot_wrapper = interact::PairDistancePotential<decltype(pairpot), float, 3> {pairpot};
    auto pair_interaction_handler =
        interact::FullPairInteractionHandler<decltype(pairpot_wrapper), float, 3> {pairpot_wrapper};

    // the FullTripletInteractionHandler takes as an argument a functor that takes
    // three points and returns a floating-point value as a result
    auto tripletpot = interact::AxilrodTellerMuto {1.0f};
    auto tripletpot_wrapper = interact::TripletDistancePotential<decltype(tripletpot), float, 3> {tripletpot};
    auto triplet_interaction_handler =
        interact::FullTripletInteractionHandler<decltype(tripletpot_wrapper), float, 3> {tripletpot_wrapper};

    using PairType = decltype(pair_interaction_handler);
    using TripletType = decltype(triplet_interaction_handler);

    const auto interaction_handler_2b3b = interact::CompositeFullInteractionHandler<float, 3, PairType, TripletType> {
        pair_interaction_handler, triplet_interaction_handler};

    const auto actual = interaction_handler_2b3b(0, worldlines[0]);

    const auto expected_direct = 2.0f * pairpot(side_length) + pairpot(std::sqrt(2.0f) * side_length)
                               + tripletpot(side_length, std::sqrt(2.0f) * side_length, side_length)
                               + tripletpot(side_length, side_length, std::sqrt(2.0f) * side_length)
                               + tripletpot(std::sqrt(2.0f) * side_length, side_length, side_length);

    const auto expected_sep_handlers =
        pair_interaction_handler(0, worldlines[0]) + triplet_interaction_handler(0, worldlines[0]);

    REQUIRE_THAT(actual, Catch::Matchers::WithinRel(expected_direct));
    REQUIRE_THAT(actual, Catch::Matchers::WithinRel(expected_sep_handlers));
}
