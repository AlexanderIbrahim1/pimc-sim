#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mathtools/interpolate/linear_interp.hpp"

TEST_CASE("trivial linear interpolation")
{
    const auto ydata = std::vector<double> {0.0, 2.0};
    const auto xmin = double {0.0};
    const auto xmax = double {1.0};

    const auto interpolator = interp::RegularLinearInterpolator {ydata, xmin, xmax};

    struct TestPair
    {
        double input {};
        double expected {};
    };

    SECTION("intercept within bounds")
    {
        auto pairs = GENERATE(TestPair {0.2, 0.4}, TestPair {0.5, 1.0}, TestPair {0.8, 1.6});

        const auto actual = interpolator(pairs.input);
        REQUIRE_THAT(pairs.expected, Catch::Matchers::WithinRel(actual));
    }

    SECTION("throws below xmin")
    {
        REQUIRE_THROWS_AS(interpolator.at(-0.5), std::runtime_error);
    }

    SECTION("throws above xmax")
    {
        REQUIRE_THROWS_AS(interpolator.at(1.5), std::runtime_error);
    }
}

TEST_CASE("less trivial linear interpolation")
{
    const auto ydata = std::vector<double> {0.0, 2.0, 1.0};
    const auto xmin = double {0.0};
    const auto xmax = double {1.0};

    const auto interpolator = interp::RegularLinearInterpolator {ydata, xmin, xmax};

    struct TestPair
    {
        double input {};
        double expected {};
    };

    SECTION("intercept within bounds")
    {
        auto pairs = GENERATE(TestPair {0.25, 1.0}, TestPair {0.5, 2.0}, TestPair {0.75, 1.5});

        const auto actual = interpolator(pairs.input);
        REQUIRE_THAT(pairs.expected, Catch::Matchers::WithinRel(actual));
    }
}

TEST_CASE("errors upon improper construction")
{
    SECTION("throw upon empty data")
    {
        const auto ydata = std::vector<double> {};

        // NOTE: REQUIRE_THROWS_AS() doesn't allow brace initialization with constructors (parsing issues)
        REQUIRE_THROWS_AS(interp::RegularLinearInterpolator(ydata, 0.0, 1.0), std::runtime_error);
    }

    SECTION("throw upon incorrectly-ordered ends")
    {
        const auto ydata = std::vector<double> {0.0, 1.0};
        const auto xmin = double {1.0};
        const auto xmin = double {0.0};

        REQUIRE_THROWS_AS(interp::RegularLinearInterpolator(ydata, xmin, xmax), std::runtime_error);
    }
}
