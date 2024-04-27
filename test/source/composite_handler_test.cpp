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

TEST_CASE("test composite full handler", "CompositeFullInteractionHandler")
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
    const auto expected = 3.0 * pairpot(side_length) + tripletpot(side_length, side_length, side_length);

    REQUIRE_THAT(actual, Catch::Matchers::WithinRel(expected));
}
