#include <array>
#include <initializer_list>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "coordinates/measure.hpp"
#include "worldline/worldline.hpp"

TEST_CASE("worldline construction", "[Worldline]")
{
    using Point = coord::Cartesian<double, 3>;

    const auto point0 = Point {1.0, 2.0, 3.0};
    const auto point1 = Point {4.0, 5.0, 6.0};
    const auto point2 = Point {7.0, 8.0, 9.0};

    SECTION("from array")
    {
        const auto arr = std::array<Point, 3> {point0, point1, point2};
        const auto wline = worldline::Worldline<double, 3> {arr};

        const auto points = wline.points();
        REQUIRE(coord::approx_eq(point0, points[0]));
        REQUIRE(coord::approx_eq(point1, points[1]));
        REQUIRE(coord::approx_eq(point2, points[2]));
    }

    SECTION("from vector")
    {
        const auto vec = std::vector<Point> {point0, point1, point2};
        const auto wline = worldline::Worldline<double, 3> {vec};

        const auto points = wline.points();
        REQUIRE(coord::approx_eq(point0, points[0]));
        REQUIRE(coord::approx_eq(point1, points[1]));
        REQUIRE(coord::approx_eq(point2, points[2]));
    }

    SECTION("from initializer list")
    {
        const auto ilist = std::initializer_list<Point> {point0, point1, point2};
        const auto wline = worldline::Worldline<double, 3> {ilist};

        const auto points = wline.points();
        REQUIRE(coord::approx_eq(point0, points[0]));
        REQUIRE(coord::approx_eq(point1, points[1]));
        REQUIRE(coord::approx_eq(point2, points[2]));
    }

    SECTION("with initializer list")
    {
        const auto wline = worldline::Worldline<double, 3> {point0, point1, point2};

        const auto points = wline.points();
        REQUIRE(coord::approx_eq(point0, points[0]));
        REQUIRE(coord::approx_eq(point1, points[1]));
        REQUIRE(coord::approx_eq(point2, points[2]));
    }
}
