#include <cstddef>
#include <sstream>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <common/buffered_writers/buffered_writer.hpp>


TEST_CASE("format_value")
{
    namespace cw = common::writers;
    using Tuple = std::tuple<std::size_t, int, double>;

    auto stream = std::stringstream {};
    const auto data = Tuple {1, 10, 2.5};

    const auto format_info = [&]() {
        const auto block_index_padding = int {5};
        const auto spacing = std::size_t {3};
        const auto integer_padding = std::array<int, 2> {5, 5};
        const auto floating_point_precision = std::array<int, 2> {8, 8};

        return cw::FormatInfo<2> {block_index_padding, spacing, integer_padding, floating_point_precision};
    }();

    cw::format_value<1, 2, Tuple>(stream, data, format_info);

    const auto actual = stream.str();

    // NOTE: the `format_value()` does not format the block index or the spacing after it;
    //       so only the 10 and 2.5 get formatted
    const auto expected = std::string{"   "} + "   10" + "   " + "2.50000000e+00";

    REQUIRE(actual == expected);
}
