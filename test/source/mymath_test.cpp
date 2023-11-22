#include <catch2/catch_test_macros.hpp>

#include "mymath.hpp"


TEST_CASE("add", "[MyMath]") {
    int x = 1;
    int y = 2;
    int expected = 3;
    int actual = mymath::add(x, y);

    REQUIRE(expected == actual);
}
