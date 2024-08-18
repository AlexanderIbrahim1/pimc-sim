#include <array>

#include <catch2/catch_test_macros.hpp>

#include "interactions/four_body/transformers.hpp"

TEST_CASE("transformers LessThanEpsilon", "[transformers]")
{
    const auto epsilon = 1.0e-4f;
    const auto comparator = impl_interact_trans::LessThanEpsilon<float> {epsilon};

    SECTION("proper less than")
    {
        const auto left = std::array<float, 6> {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
        const auto right = std::array<float, 6> {1.0f, 2.0f, 3.0f + 1.0e-3f, 4.0f, 5.0f, 6.0f};

        REQUIRE(comparator(left, right));
    }

    SECTION("equal due to small epsilon difference")
    {
        const auto left = std::array<float, 6> {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
        const auto right = std::array<float, 6> {1.0f, 2.0f, 3.0f + 1.0e-5f, 4.0f, 5.0f, 6.0f};

        REQUIRE(!comparator(left, right));
    }
}
