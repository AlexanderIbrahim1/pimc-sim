#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "common/writer_utils.hpp"
#include "mathtools/histogram/histogram.hpp"
#include "mathtools/io/histogram.hpp"

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

TEST_CASE("write histogram", "[Histogram]")
{
    auto histogram = mathtools::Histogram<double> {0.0, 1.0, 5};
    histogram.add(0.1, 2);
    histogram.add(0.3, 5);
    histogram.add(0.5, 7);
    histogram.add(0.9, 3);

    auto actual = std::stringstream {};
    mathtools::io::write_histogram(actual, histogram);

    common::writers::skip_lines_starting_with(actual, '#');

    // check the policy
    auto policy_key = int {};
    actual >> policy_key;
    auto policy = static_cast<mathtools::OutOfRangePolicy>(policy_key);
    REQUIRE(policy == histogram.policy());

    // check the number of bins
    auto n_bins = std::size_t {};
    actual >> n_bins;
    REQUIRE(n_bins == histogram.bins().size());

    // check the minimum and maximum histogram bounds
    auto min = double {};
    actual >> min;
    REQUIRE_THAT(histogram.min(), Catch::Matchers::WithinRel(min));

    auto max = double {};
    actual >> max;
    REQUIRE_THAT(histogram.max(), Catch::Matchers::WithinRel(max));

    // check the entries
    auto count = std::uint64_t {};
    auto expected_counts = std::vector<std::uint64_t> {};
    for (std::size_t i {0}; i < 5; ++i) {
        actual >> count;
        expected_counts.push_back(count);
    }

    REQUIRE(histogram.bins() == expected_counts);
}

TEST_CASE("write and read histogram", "[Histogram]")
{
    auto out_histogram = mathtools::Histogram<double> {0.0, 1.0, 5};
    out_histogram.add(0.1, 2);
    out_histogram.add(0.3, 5);
    out_histogram.add(0.5, 7);
    out_histogram.add(0.9, 3);

    auto actual = std::stringstream {};
    mathtools::io::write_histogram(actual, out_histogram);

    const auto in_histogram = mathtools::io::read_histogram<double>(actual);

    REQUIRE(out_histogram.policy() == in_histogram.policy());
    REQUIRE(out_histogram.size() == in_histogram.size());
    REQUIRE_THAT(out_histogram.min(), Catch::Matchers::WithinRel(in_histogram.min()));
    REQUIRE_THAT(out_histogram.max(), Catch::Matchers::WithinRel(in_histogram.max()));

    const auto& out_bins = out_histogram.bins();
    const auto& in_bins = in_histogram.bins();
    for (std::size_t i {0}; i < 5; ++i) {
        REQUIRE(out_bins[i] == in_bins[i]);
    }
}
