#include <catch2/catch_test_macros.hpp>

#include "pimc/bisection_level_manager.hpp"

TEST_CASE("bisection level manager")
{
    SECTION("max level 1")
    {
        const auto blmanager = pimc::BisectionLevelManager {1, 0, 1000};

        auto it = std::begin(blmanager.triplets(0));

        REQUIRE(*it == pimc::BisectionTriplets {0, 1, 2});
    }

    SECTION("max level 2")
    {
        const auto blmanager = pimc::BisectionLevelManager {2, 0, 1000};

        auto it0 = std::begin(blmanager.triplets(0));
        auto it1 = std::begin(blmanager.triplets(1));

        REQUIRE(*it0++ == pimc::BisectionTriplets {0, 2, 4});
        REQUIRE(*it1++ == pimc::BisectionTriplets {0, 1, 2});
        REQUIRE(*it1++ == pimc::BisectionTriplets {2, 3, 4});
    }

    SECTION("max level 3")
    {
        const auto blmanager = pimc::BisectionLevelManager {3, 0, 1000};

        auto it0 = std::begin(blmanager.triplets(0));
        auto it1 = std::begin(blmanager.triplets(1));
        auto it2 = std::begin(blmanager.triplets(2));

        REQUIRE(*it0++ == pimc::BisectionTriplets {0, 4, 8});
        REQUIRE(*it1++ == pimc::BisectionTriplets {0, 2, 4});
        REQUIRE(*it1++ == pimc::BisectionTriplets {4, 6, 8});
        REQUIRE(*it2++ == pimc::BisectionTriplets {0, 1, 2});
        REQUIRE(*it2++ == pimc::BisectionTriplets {2, 3, 4});
        REQUIRE(*it2++ == pimc::BisectionTriplets {4, 5, 6});
        REQUIRE(*it2++ == pimc::BisectionTriplets {6, 7, 8});
    }

    SECTION("max level 2, with offset and modulo")
    {
        const auto blmanager = pimc::BisectionLevelManager {2, 5, 8};

        auto it0 = std::begin(blmanager.triplets(0));
        auto it1 = std::begin(blmanager.triplets(1));

        REQUIRE(*it0++ == pimc::BisectionTriplets {5, 7, 1});
        REQUIRE(*it1++ == pimc::BisectionTriplets {5, 6, 7});
        REQUIRE(*it1++ == pimc::BisectionTriplets {7, 0, 1});
    }
}
