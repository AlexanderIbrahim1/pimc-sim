#include <cstddef>
#include <filesystem>
#include <sstream>
#include <string>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <common/buffered_writers/buffered_writer.hpp>

#include "../test_utils/test_utils.hpp"


namespace cw = common::writers;

constexpr auto default_format_info1() noexcept -> cw::FormatInfo<1>
{
    const auto block_index_padding = int {5};
    const auto spacing = std::size_t {3};
    const auto integer_padding = std::array<int, 1> {5};
    const auto floating_point_precision = std::array<int, 1> {8};

    return cw::FormatInfo<1> {block_index_padding, spacing, integer_padding, floating_point_precision};
}

constexpr auto default_format_info2() noexcept -> cw::FormatInfo<2>
{
    const auto block_index_padding = int {5};
    const auto spacing = std::size_t {3};
    const auto integer_padding = std::array<int, 2> {5, 5};
    const auto floating_point_precision = std::array<int, 2> {8, 8};

    return cw::FormatInfo<2> {block_index_padding, spacing, integer_padding, floating_point_precision};
}

constexpr auto default_format_info3() noexcept -> cw::FormatInfo<3>
{
    const auto block_index_padding = int {5};
    const auto spacing = std::size_t {3};
    const auto integer_padding = std::array<int, 3> {5, 5, 5};
    const auto floating_point_precision = std::array<int, 3> {8, 8, 8};

    return cw::FormatInfo<3> {block_index_padding, spacing, integer_padding, floating_point_precision};
}

TEST_CASE("format_value")
{
    namespace cw = common::writers;

    const auto format_info1 = default_format_info1();
    const auto format_info2 = default_format_info2();
    const auto format_info3 = default_format_info3();

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

TEST_CASE("BufferedStreamValueWriter")
{
    namespace cw = common::writers;
    using st = std::string;

    const auto format_info1 = default_format_info1();
    const auto format_info2 = default_format_info2();

    SECTION("int, int, 3 lines")
    {
        auto buffered_writer = cw::BufferedStreamValueWriter<int, int> {};
        buffered_writer.accumulate({0, 10, 20});
        buffered_writer.accumulate({1, 30, 40});
        buffered_writer.accumulate({2, 50, 60});

        auto stream = std::stringstream {};
        buffered_writer.write_and_clear(stream, format_info2);
        const auto actual = stream.str();

        // clang-format off
        const auto expected = st{"00000"} + st{"   "} + st{"   10"} + st{"   "} + st{"   20"} + '\n'
                            + st{"00001"} + st{"   "} + st{"   30"} + st{"   "} + st{"   40"} + '\n'
                            + st{"00002"} + st{"   "} + st{"   50"} + st{"   "} + st{"   60"} + '\n';
        // clang-format on

        REQUIRE(actual == expected);

        // make sure the buffer is empty now
        auto post_write_stream = std::stringstream {};
        buffered_writer.write_and_clear(post_write_stream, format_info2);
        const auto post_actual = post_write_stream.str();

        const auto post_expected = std::string {""};

        REQUIRE(post_actual == post_expected);
    }

    SECTION("double, 2 lines")
    {
        auto buffered_writer = cw::BufferedStreamValueWriter<double> {};
        buffered_writer.accumulate({0, 123.456});
        buffered_writer.accumulate({1, 654.321});

        auto stream = std::stringstream {};
        buffered_writer.write_and_clear(stream, format_info1);
        const auto actual = stream.str();

        // clang-format off
        const auto expected = st{"00000"} + st{"   "} + st{"1.23456000e+02"} + '\n'
                            + st{"00001"} + st{"   "} + st{"6.54321000e+02"} + '\n';
        // clang-format on

        REQUIRE(actual == expected);

        // make sure the buffer is empty now
        auto post_write_stream = std::stringstream {};
        buffered_writer.write_and_clear(post_write_stream, format_info1);
        const auto post_actual = post_write_stream.str();

        const auto post_expected = std::string {""};

        REQUIRE(post_actual == post_expected);
    }
}

TEST_CASE("BlockValueWriter")
{
    namespace fs = std::filesystem;
    namespace cw = common::writers;

    SECTION("single_value_example0")
    {
        const auto rel_dirpath = fs::path{"test"} / "test_io" / "buffered_writer" / "single_value_example0";
        const auto filename = "test_single_value_example0.txt";
        const auto abs_filepath = test_utils::resolve_project_path(rel_dirpath) / filename;

        const auto header = std::string {"# dummy header\n"};

        auto writer = cw::BlockValueWriter<int> {abs_filepath, header};

        REQUIRE(!fs::exists(abs_filepath));
    }
    // PLAN
    // create an instance
    // define (but don't explicitly create) a file in the new directory for the I/O testing
    //   - (unfortunately, it looks like I need to perform I/O to test this class!)
    // CHECK: file doesn't exist yet
    // accumulate some data
    // CHECK: file doesn't exist yet
    // accumulate some more data
    // CHECK: file doesn't exist yet
    // write_and_clear
    // CHECK: file exists
    // CHECK: file contents are as expected
    // accumulate some more data
    // CHECK: file exists
    // CHECK: file contents are unchanged
    // accumulate some more data
    // CHECK: file exists
    // CHECK: file contents are changed

    // to perform these tests
    // - new function to check the file contents
}
