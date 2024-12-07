#include <cstddef>
#include <sstream>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <common/buffered_writers/buffered_writer.hpp>


TEST_CASE("format_value")
{
    namespace cw = common::writers;

    const auto format_info1 = [&]() {
        const auto block_index_padding = int {5};
        const auto spacing = std::size_t {3};
        const auto integer_padding = std::array<int, 1> {5};
        const auto floating_point_precision = std::array<int, 1> {8};

        return cw::FormatInfo<1> {block_index_padding, spacing, integer_padding, floating_point_precision};
    }();

    const auto format_info2 = [&]() {
        const auto block_index_padding = int {5};
        const auto spacing = std::size_t {3};
        const auto integer_padding = std::array<int, 2> {5, 5};
        const auto floating_point_precision = std::array<int, 2> {8, 8};

        return cw::FormatInfo<2> {block_index_padding, spacing, integer_padding, floating_point_precision};
    }();

    const auto format_info3 = [&]() {
        const auto block_index_padding = int {5};
        const auto spacing = std::size_t {3};
        const auto integer_padding = std::array<int, 3> {5, 5, 5};
        const auto floating_point_precision = std::array<int, 3> {8, 8, 8};

        return cw::FormatInfo<3> {block_index_padding, spacing, integer_padding, floating_point_precision};
    }();


    SECTION("int")
    {
        using Tuple = std::tuple<std::size_t, int>;

        auto stream = std::stringstream {};
        const auto data = Tuple {1, 10};
        cw::format_value<1, 1, Tuple>(stream, data, format_info1);
        const auto actual = stream.str();

        // clang-format off
        const auto expected = std::string{"   "} + std::string{"   10"};
        // clang-format on

        REQUIRE(actual == expected);
    }

    SECTION("int, double")
    {
        using Tuple = std::tuple<std::size_t, int, double>;

        auto stream = std::stringstream {};
        const auto data = Tuple {1, 10, 2.5};
        cw::format_value<1, 2, Tuple>(stream, data, format_info2);
        const auto actual = stream.str();

        // clang-format off
        const auto expected = std::string{"   "} + std::string{"   10"} + \
                              std::string{"   "} + std::string{"2.50000000e+00"};
        // clang-format on

        REQUIRE(actual == expected);
    }

    SECTION("int, int")
    {
        using Tuple = std::tuple<std::size_t, int, int>;

        auto stream = std::stringstream {};
        const auto data = Tuple {1, 10, 123};
        cw::format_value<1, 2, Tuple>(stream, data, format_info2);
        const auto actual = stream.str();

        // clang-format off
        const auto expected = std::string{"   "} + std::string{"   10"} + \
                              std::string{"   "} + std::string{"  123"};
        // clang-format on

        REQUIRE(actual == expected);
    }

    SECTION("double, double")
    {
        using Tuple = std::tuple<std::size_t, double, double>;

        auto stream = std::stringstream {};
        const auto data = Tuple {1, 123.456, 654.321};
        cw::format_value<1, 2, Tuple>(stream, data, format_info2);
        const auto actual = stream.str();

        // clang-format off
        const auto expected = std::string{"   "} + std::string{"1.23456000e+02"} + \
                              std::string{"   "} + std::string{"6.54321000e+02"};
        // clang-format on

        REQUIRE(actual == expected);
    }

    SECTION("double, int")
    {
        using Tuple = std::tuple<std::size_t, double, int>;

        auto stream = std::stringstream {};
        const auto data = Tuple {1, 2.5, 15};
        cw::format_value<1, 2, Tuple>(stream, data, format_info2);
        const auto actual = stream.str();

        // clang-format off
        const auto expected = std::string{"   "} + std::string{"2.50000000e+00"} + \
                              std::string{"   "} + std::string{"   15"};
        // clang-format on

        REQUIRE(actual == expected);
    }

    SECTION("int, int, int")
    {
        using Tuple = std::tuple<std::size_t, int, int, int>;

        auto stream = std::stringstream {};
        const auto data = Tuple {1, 10, 123, 456};

        cw::format_value<1, 3, Tuple>(stream, data, format_info3);
        const auto actual = stream.str();

        // clang-format off
        const auto expected = std::string{"   "} + std::string{"   10"} + \
                              std::string{"   "} + std::string{"  123"} + \
                              std::string{"   "} + std::string{"  456"};
        // clang-format on

        REQUIRE(actual == expected);
    }
}
