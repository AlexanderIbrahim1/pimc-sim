#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"


TEST_CASE("construction", "[Cartesian]") {
    const auto c = coord::Cartesian<double, 3>(1.0, 2.0, 3.0);
    REQUIRE_THAT(c[0], Catch::Matchers::WithinRel(1.0));
    REQUIRE_THAT(c[1], Catch::Matchers::WithinRel(2.0));
    REQUIRE_THAT(c[2], Catch::Matchers::WithinRel(3.0));
}