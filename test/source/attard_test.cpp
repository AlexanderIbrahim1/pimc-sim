#include <array>
#include <cmath>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/attard/three_body.hpp"
#include "coordinates/box_sides.hpp"
#include "coordinates/cartesian.hpp"

TEST_CASE("basic three-body attard", "[three_body_attard_side_lengths]")
{
    using Point = coord::Cartesian<double, 2>;

    const auto points = std::array<Point, 3> {
        Point {-0.1, 0.0},
         Point {0.1,  0.0},
         Point {0.0,  0.1}
    };

    const auto box = coord::BoxSides<double, 2> {1.0, 1.0};

    const auto expected = std::array<double, 3> {0.2, 0.1 * std::sqrt(2.0), 0.1 * std::sqrt(2.0)};
    const auto actual = coord::three_body_attard_side_lengths(points, box);

    REQUIRE_THAT(expected[0], Catch::Matchers::WithinRel(actual[0]));
    REQUIRE_THAT(expected[1], Catch::Matchers::WithinRel(actual[1]));
    REQUIRE_THAT(expected[2], Catch::Matchers::WithinRel(actual[2]));
}
