#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "coordinates/constants.hpp"
#include "coordinates/measure.hpp"

TEST_CASE("construction", "[Cartesian]")
{
    const auto c = coord::Cartesian<double, 3>(1.0, 2.0, 3.0);
    REQUIRE_THAT(c[0], Catch::Matchers::WithinRel(1.0));
    REQUIRE_THAT(c[1], Catch::Matchers::WithinRel(2.0));
    REQUIRE_THAT(c[2], Catch::Matchers::WithinRel(3.0));
}

TEST_CASE("default constructor", "[Cartesian]")
{
    auto c = coord::Cartesian<double, 3>();
    REQUIRE_THAT(c[0], Catch::Matchers::WithinRel(0.0));
    REQUIRE_THAT(c[1], Catch::Matchers::WithinRel(0.0));
    REQUIRE_THAT(c[2], Catch::Matchers::WithinRel(0.0));
}

TEST_CASE("change individual coordinates, Float& operator[]", "[Cartesian]")
{
    auto c = coord::Cartesian<double, 3>(1.0, 2.0, 3.0);
    c[0] = 3.0;
    REQUIRE_THAT(c[0], Catch::Matchers::WithinRel(3.0));
}

TEST_CASE("shift_coord", "[Cartesian]")
{
    auto c = coord::Cartesian<double, 2>();

    REQUIRE_THAT(c[0], Catch::Matchers::WithinRel(0.0));
    REQUIRE_THAT(c[1], Catch::Matchers::WithinRel(0.0));

    c.shift_coord(0, 0.5);
    c.shift_coord(1, -0.25);

    REQUIRE_THAT(c[0], Catch::Matchers::WithinRel(0.5));
    REQUIRE_THAT(c[1], Catch::Matchers::WithinRel(-0.25));
}

TEST_CASE("shift_coord_checked", "[Cartesian]")
{
    auto c = coord::Cartesian<double, 2>();

    SECTION("shift_coord_checked (success)")
    {
        c.shift_coord_checked(0, 0.5);
        c.shift_coord_checked(1, -0.25);

        REQUIRE_THAT(c[0], Catch::Matchers::WithinRel(0.5));
        REQUIRE_THAT(c[1], Catch::Matchers::WithinRel(-0.25));
    }

    SECTION("shift_coord_checked (throw)")
    {
        REQUIRE_THROWS_AS(c.shift_coord_checked(2, 1.0), std::runtime_error);
    }
}

TEST_CASE("at (Float)", "[Cartesian]")
{
    auto c = coord::Cartesian<double, 2>(1.1, -2.05);

    SECTION("at (Float) (success)")
    {
        REQUIRE_THAT(c.at(0), Catch::Matchers::WithinRel(1.1));
        REQUIRE_THAT(c.at(1), Catch::Matchers::WithinRel(-2.05));
    }

    SECTION("at (Float) (throws)")
    {
        REQUIRE_THROWS_AS(c.at(2), std::runtime_error);
    }
}

TEST_CASE("at (Float&)", "[Cartesian]")
{
    auto c = coord::Cartesian<double, 2>(1.1, -2.05);

    SECTION("at (Float&) (success)")
    {
        c.at(0, -1.0);
        c.at(1, 3.5);
        REQUIRE_THAT(c.at(0), Catch::Matchers::WithinRel(-1.0));
        REQUIRE_THAT(c.at(1), Catch::Matchers::WithinRel(3.5));
    }

    SECTION("at (Float) (throws)")
    {
        REQUIRE_THROWS_AS(c.at(2, 1.5), std::runtime_error);
    }
}

TEST_CASE("as_string with a 2D Cartesian", "[Cartesian]")
{
    SECTION("both positive")
    {
        const auto c = coord::Cartesian<double, 2>(1.23, 4.56);
        REQUIRE(c.as_string() == "( 1.230000,  4.560000)");
    }

    SECTION("left positive, right negative")
    {
        const auto c = coord::Cartesian<double, 2>(1.23, -4.56);
        REQUIRE(c.as_string() == "( 1.230000, -4.560000)");
    }

    SECTION("left negative, right positive")
    {
        const auto c = coord::Cartesian<double, 2>(-1.23, 4.56);
        REQUIRE(c.as_string() == "(-1.230000,  4.560000)");
    }

    SECTION("both negative")
    {
        const auto c = coord::Cartesian<double, 2>(-1.23, -4.56);
        REQUIRE(c.as_string() == "(-1.230000, -4.560000)");
    }
}

TEST_CASE("addition", "[Cartesian]")
{
    const auto p0 = coord::Cartesian<double, 3>(1.0, 2.0, 3.0);
    const auto p1 = coord::Cartesian<double, 3>(4.0, 5.0, 6.0);

    const auto p2 = p0 + p1;
    REQUIRE_THAT(p2[0], Catch::Matchers::WithinRel(5.0));
    REQUIRE_THAT(p2[1], Catch::Matchers::WithinRel(7.0));
    REQUIRE_THAT(p2[2], Catch::Matchers::WithinRel(9.0));
}

TEST_CASE("subtraction", "[Cartesian]")
{
    const auto p0 = coord::Cartesian<double, 3>(1.0, 2.0, 3.0);
    const auto p1 = coord::Cartesian<double, 3>(4.0, 6.0, -8.0);

    const auto p2 = p0 - p1;
    REQUIRE_THAT(p2[0], Catch::Matchers::WithinRel(-3.0));
    REQUIRE_THAT(p2[1], Catch::Matchers::WithinRel(-4.0));
    REQUIRE_THAT(p2[2], Catch::Matchers::WithinRel(11.0));
}

TEST_CASE("multiplication", "[Cartesian]")
{
    const auto point = coord::Cartesian<double, 3>(1.0, 2.0, 3.0);

    SECTION("multiplication by 2.0, from left")
    {
        const auto actual_point = 2.0 * point;
        const auto expect_point = coord::Cartesian<double, 3>(2.0, 4.0, 6.0);

        REQUIRE(coord::approx_eq(actual_point, expect_point));
    }

    SECTION("multiplication by 2.0, from right")
    {
        const auto actual_point = point * 2.0;
        const auto expect_point = coord::Cartesian<double, 3>(2.0, 4.0, 6.0);

        REQUIRE(coord::approx_eq(actual_point, expect_point));
    }

    SECTION("multiplication by zero")
    {
        const auto actual_point = point * 0.0;
        const auto expect_point = coord::Cartesian<double, 3>(0.0, 0.0, 0.0);

        REQUIRE(coord::approx_eq(actual_point, expect_point));
    }
}

TEST_CASE("division", "[Cartesian]")
{
    const auto point = coord::Cartesian<double, 3>(1.0, 2.0, 3.0);

    SECTION("division by 2.0")
    {
        const auto actual_point = point / 2.0;
        const auto expect_point = coord::Cartesian<double, 3>(0.5, 1.0, 1.5);

        REQUIRE(coord::approx_eq(actual_point, expect_point));
    }
}

TEST_CASE("origin", "[Cartesian]")
{
    const auto actual_origin = coord::Cartesian<double, 2>::origin();
    const auto expect_origin = coord::Cartesian<double, 2>(0.0, 0.0);

    REQUIRE(coord::approx_eq(actual_origin, expect_origin));
}

TEST_CASE("operator-", "[Cartesian]")
{
    const auto original = coord::Cartesian<double, 3>(1.1, 2.2, 3.3);
    const auto negated = -original;

    REQUIRE_THAT(negated[0], Catch::Matchers::WithinRel(-1.1));
    REQUIRE_THAT(negated[1], Catch::Matchers::WithinRel(-2.2));
    REQUIRE_THAT(negated[2], Catch::Matchers::WithinRel(-3.3));
}

TEST_CASE("operator+", "[Cartesian]")
{
    auto original = coord::Cartesian<double, 3>(1.1, 2.2, 3.3);
    const auto positive = +original;

    REQUIRE_THAT(positive[0], Catch::Matchers::WithinRel(1.1));
    REQUIRE_THAT(positive[1], Catch::Matchers::WithinRel(2.2));
    REQUIRE_THAT(positive[2], Catch::Matchers::WithinRel(3.3));

    // make sure that we have a copy, and not the original data
    original[0] = 4.56;
    REQUIRE_THAT(original[0], Catch::Matchers::WithinRel(4.56));
    REQUIRE_THAT(positive[0], Catch::Matchers::WithinRel(1.1));
}
