#include <catch2/catch_test_macros.hpp>

#include "mathtools/grid/grid2d.hpp"

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
