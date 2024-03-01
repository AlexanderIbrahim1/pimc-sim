#include <cstddef>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "mathtools/histogram/histogram.hpp"

TEST_CASE("basic histogram", "[Histogram]")
{
    SECTION("throw when min and max are in the incorrect order")
    {
        REQUIRE_THROWS_AS(mathtools::Histogram<double>(1.0, 0.0, 100), std::runtime_error);
    }

    SECTION("throw if the number of bins is 0")
    {
        REQUIRE_THROWS_AS(mathtools::Histogram<double>(0.0, 1.0, 0), std::runtime_error);
    }

    SECTION("throws if we go out of bounds when setting the policy to throw")
    {
        auto histogram = mathtools::Histogram<double>(0.0, 1.0, 10, mathtools::OutOfRangePolicy::THROW);
        REQUIRE_THROWS_AS(histogram.add(-0.5), std::runtime_error);
        REQUIRE_THROWS_AS(histogram.add(1.0), std::runtime_error);
        REQUIRE_THROWS_AS(histogram.add(1.5), std::runtime_error);
    }

    SECTION("put entries in the correct bins")
    {
        auto histogram = mathtools::Histogram<double>(0.0, 1.0, 5);
        histogram.add(0.1);   // into 0
        histogram.add(0.15);  // into 0
        histogram.add(0.05);  // into 0
        histogram.add(0.25);  // into 1
        histogram.add(0.22);  // into 1
        histogram.add(0.45);  // into 2
        histogram.add(0.9);   // into 4

        REQUIRE(histogram.bins() == std::vector<std::uint64_t> {3, 2, 1, 0, 1});
    }

    SECTION("gives true/false if it goes out of bounds")
    {
        auto histogram = mathtools::Histogram<double>(0.0, 1.0, 5);
        REQUIRE(histogram.add(0.1));
        REQUIRE(histogram.add(0.3));
        REQUIRE(!histogram.add(-0.1));
        REQUIRE(!histogram.add(1.1));
    }

    SECTION("resets properly")
    {
        auto histogram = mathtools::Histogram<double>(0.0, 1.0, 5);
        histogram.add(0.1);   // into 0
        histogram.add(0.15);  // into 0
        histogram.add(0.35);  // into 1

        REQUIRE(histogram.bins() == std::vector<std::uint64_t> {2, 1, 0, 0, 0});

        histogram.reset();

        REQUIRE(histogram.bins() == std::vector<std::uint64_t> {0, 0, 0, 0, 0});
    }
}
