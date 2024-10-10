#include <algorithm>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "mathtools/grid/grid2d.hpp"

constexpr auto is_equal_along_col(
    const mathtools::Grid2D<int>& grid,
    std::size_t i_col,
    const std::vector<int>& expected
) -> bool
{
    auto actual = std::vector<int> {};
    actual.reserve(grid.n_cols());

    for (auto elem : grid.iterator_along_col(i_col)) {
        actual.push_back(elem);
    }

    return std::equal(actual.begin(), actual.end(), expected.begin());
}

constexpr auto is_equal_along_row(
    const mathtools::Grid2D<int>& grid,
    std::size_t i_row,
    const std::vector<int>& expected
) -> bool
{
    auto actual = std::vector<int> {};
    actual.reserve(grid.n_rows());

    for (auto elem : grid.iterator_along_row(i_row)) {
        actual.push_back(elem);
    }

    return std::equal(actual.begin(), actual.end(), expected.begin());
}

TEST_CASE("basic Grid2D test")
{
    auto grid = mathtools::Grid2D<int>(2, 3);

    grid.set(0, 0, 1);
    grid.set(0, 1, 2);
    grid.set(0, 2, 3);
    grid.set(1, 0, 4);
    grid.set(1, 1, 5);
    grid.set(1, 2, 6);

    REQUIRE(grid.get(0, 0) == 1);
    REQUIRE(grid.get(0, 1) == 2);
    REQUIRE(grid.get(0, 2) == 3);
    REQUIRE(grid.get(1, 0) == 4);
    REQUIRE(grid.get(1, 1) == 5);
    REQUIRE(grid.get(1, 2) == 6);
}

TEST_CASE("grid iteration")
{
    const auto n_rows = std::size_t {3};
    const auto n_cols = std::size_t {4};
    auto grid = mathtools::Grid2D<int>(n_rows, n_cols);

    auto count = int {0};
    for (std::size_t i_row {0}; i_row < 3; ++i_row) {
        for (std::size_t i_col {0}; i_col < 4; ++i_col) {
            grid.set(i_row, i_col, count);
            ++count;
        }
    }

    // const auto col_iter = grid.iterator_along_col(0);
    // const auto actual = std::vector<int> {col_iter.begin(), col_iter.end()};
    // const auto expected = std::vector<int> {0, 4, 8};

    // REQUIRE(actual == expected);

    // The grid we just created looks like this:
    //  0  1  2  3
    //  4  5  6  7
    //  8  9 10 11
    REQUIRE(is_equal_along_col(grid, 0, {0, 4, 8}));
    REQUIRE(is_equal_along_col(grid, 1, {1, 5, 9}));
    REQUIRE(is_equal_along_col(grid, 2, {2, 6, 10}));
    REQUIRE(is_equal_along_col(grid, 3, {3, 7, 11}));

    REQUIRE(is_equal_along_row(grid, 0, {0, 1, 2, 3}));
    REQUIRE(is_equal_along_row(grid, 1, {4, 5, 6, 7}));
    REQUIRE(is_equal_along_row(grid, 2, {8, 9, 10, 11}));
}
