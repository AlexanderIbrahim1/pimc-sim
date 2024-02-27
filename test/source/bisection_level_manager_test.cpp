#include <catch2/catch_test_macros.hpp>

#include "pimc/bisection_level_manager.hpp"


TEST_CASE("bisection level manager") {
    SECTION("max level 1") {
        const auto blmanager = pimc::BisectionLevelManager {1};

        auto it = std::begin(blmanager.triplets(0));

        REQUIRE(*it == pimc::BisectionTriplets{0, 1, 2});
    }

    SECTION("max level 2") {
        const auto blmanager = pimc::BisectionLevelManager {2};

        auto it0 = std::begin(blmanager.triplets(0));
        auto it1 = std::begin(blmanager.triplets(1));

        REQUIRE(*it0++ == pimc::BisectionTriplets{0, 2, 4});
        REQUIRE(*it1++ == pimc::BisectionTriplets{0, 1, 2});
        REQUIRE(*it1++ == pimc::BisectionTriplets{2, 3, 4});
    }

    SECTION("max level 3") {
        const auto blmanager = pimc::BisectionLevelManager {3};

        auto it0 = std::begin(blmanager.triplets(0));
        auto it1 = std::begin(blmanager.triplets(1));
        auto it2 = std::begin(blmanager.triplets(2));

        REQUIRE(*it0++ == pimc::BisectionTriplets{0, 4, 8});
        REQUIRE(*it1++ == pimc::BisectionTriplets{0, 2, 4});
        REQUIRE(*it1++ == pimc::BisectionTriplets{4, 6, 8});
        REQUIRE(*it2++ == pimc::BisectionTriplets{0, 1, 2});
        REQUIRE(*it2++ == pimc::BisectionTriplets{2, 3, 4});
        REQUIRE(*it2++ == pimc::BisectionTriplets{4, 5, 6});
        REQUIRE(*it2++ == pimc::BisectionTriplets{6, 7, 8});
    }
}
