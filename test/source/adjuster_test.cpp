#include <stdexcept>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "pimc/adjusters/single_value_adjuster.hpp"
#include "pimc/trackers/move_success_tracker.hpp"

TEST_CASE("basic single value adjustment", "[SingleValueAdjuster]")
{
    const auto abs_adjustment = double {0.1};
    const auto current = double {5.0};

    SECTION("negative direction")
    {
        const auto direction = pimc::DirectionIfAcceptTooLow::NEGATIVE;
        const auto range = pimc::AcceptPercentageRange {0.3, 0.8, direction};
        const auto move_adjuster = pimc::SingleValueMoveAdjuster<double> {range, abs_adjustment};

        SECTION("acceptance rate too low")
        {
            // tracker with 10 % acceptance rate
            auto tracker = pimc::MoveSuccessTracker {};
            tracker.add_accept(10);
            tracker.add_reject(90);

            const auto expected = current - abs_adjustment;
            const auto actual = move_adjuster.adjust_step(current, tracker);

            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("acceptance rate too high")
        {
            // tracker with 90% acceptance rate
            auto tracker = pimc::MoveSuccessTracker {};
            tracker.add_accept(90);
            tracker.add_reject(10);

            const auto expected = current + abs_adjustment;
            const auto actual = move_adjuster.adjust_step(current, tracker);

            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("acceptance rate within acceptable range")
        {
            // tracker with 50% acceptance rate
            auto tracker = pimc::MoveSuccessTracker {};
            tracker.add_accept(50);
            tracker.add_reject(50);

            const auto expected = current;
            const auto actual = move_adjuster.adjust_step(current, tracker);

            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }
    }

    SECTION("positive direction")
    {
        const auto direction = pimc::DirectionIfAcceptTooLow::POSITIVE;
        const auto range = pimc::AcceptPercentageRange {0.3, 0.8, direction};
        const auto move_adjuster = pimc::SingleValueMoveAdjuster<double> {range, abs_adjustment};

        SECTION("acceptance rate too low")
        {
            // tracker with 10 % acceptance rate
            auto tracker = pimc::MoveSuccessTracker {};
            tracker.add_accept(10);
            tracker.add_reject(90);

            const auto expected = current + abs_adjustment;
            const auto actual = move_adjuster.adjust_step(current, tracker);

            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("acceptance rate too high")
        {
            // tracker with 90% acceptance rate
            auto tracker = pimc::MoveSuccessTracker {};
            tracker.add_accept(90);
            tracker.add_reject(10);

            const auto expected = current - abs_adjustment;
            const auto actual = move_adjuster.adjust_step(current, tracker);

            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("acceptance rate within acceptable range")
        {
            // tracker with 50% acceptance rate
            auto tracker = pimc::MoveSuccessTracker {};
            tracker.add_accept(50);
            tracker.add_reject(50);

            const auto expected = current;
            const auto actual = move_adjuster.adjust_step(current, tracker);

            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }
    }
}

TEST_CASE("accept percentage range")
{
    const auto direction = pimc::DirectionIfAcceptTooLow::NEGATIVE;  // doesn't affect tests
    SECTION("exception if below 0.0 and/or above 1.0")
    {
        REQUIRE_THROWS_AS(pimc::AcceptPercentageRange(-0.1, 0.5, direction), std::runtime_error);
        REQUIRE_THROWS_AS(pimc::AcceptPercentageRange(0.1, -0.5, direction), std::runtime_error);
        REQUIRE_THROWS_AS(pimc::AcceptPercentageRange(-0.1, -0.5, direction), std::runtime_error);
        REQUIRE_THROWS_AS(pimc::AcceptPercentageRange(1.5, 0.5, direction), std::runtime_error);
        REQUIRE_THROWS_AS(pimc::AcceptPercentageRange(0.5, 1.5, direction), std::runtime_error);
        REQUIRE_THROWS_AS(pimc::AcceptPercentageRange(1.5, 1.5, direction), std::runtime_error);
        REQUIRE_THROWS_AS(pimc::AcceptPercentageRange(1.2, -0.5, direction), std::runtime_error);
    }

    SECTION("except if incorrect order")
    {
        REQUIRE_THROWS_AS(pimc::AcceptPercentageRange(0.7, 0.5, direction), std::runtime_error);
    }
}

TEST_CASE("adjustment with limits")
{
    const auto direction = pimc::DirectionIfAcceptTooLow::NEGATIVE;
    const auto range = pimc::AcceptPercentageRange {0.3, 0.8, direction};
    const auto abs_adjustment = double {0.1};

    auto move_tracker_too_high = pimc::MoveSuccessTracker {};
    move_tracker_too_high.add_accept(90);
    move_tracker_too_high.add_reject(10);

    auto move_tracker_too_low = pimc::MoveSuccessTracker {};
    move_tracker_too_low.add_accept(10);
    move_tracker_too_low.add_reject(90);

    auto move_tracker_just_right = pimc::MoveSuccessTracker {};
    move_tracker_just_right.add_accept(50);
    move_tracker_just_right.add_reject(50);

    const auto both_limits = pimc::MoveLimits<double> {1.0, 5.0};
    const auto lower_limits = pimc::MoveLimits<double> {1.0, std::nullopt};
    const auto upper_limits = pimc::MoveLimits<double> {std::nullopt, 5.0};

    SECTION("both limits")
    {
        const auto move_adjuster = pimc::SingleValueMoveAdjuster<double> {range, abs_adjustment, both_limits};
        SECTION("bounded from below")
        {
            const auto current = double {1.05};
            const auto expected = double {1.0};
            const auto actual = move_adjuster.adjust_step(current, move_tracker_too_low);
            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("bounded from above")
        {
            const auto current = double {4.95};
            const auto expected = double {5.0};
            const auto actual = move_adjuster.adjust_step(current, move_tracker_too_high);
            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("bounded not applied")
        {
            const auto current = double {2.5};
            const auto expected = double {2.6};
            const auto actual = move_adjuster.adjust_step(current, move_tracker_too_high);
            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }
    }

    SECTION("lower limits")
    {
        const auto move_adjuster = pimc::SingleValueMoveAdjuster<double> {range, abs_adjustment, lower_limits};
        SECTION("bounded from below")
        {
            const auto current = double {1.05};
            const auto expected = double {1.0};
            const auto actual = move_adjuster.adjust_step(current, move_tracker_too_low);
            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("not bounded from above")
        {
            const auto current = double {4.95};
            const auto expected = double {5.05};
            const auto actual = move_adjuster.adjust_step(current, move_tracker_too_high);
            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("bounded not applied")
        {
            const auto current = double {2.5};
            const auto expected = double {2.6};
            const auto actual = move_adjuster.adjust_step(current, move_tracker_too_high);
            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }
    }

    SECTION("upper limits")
    {
        const auto move_adjuster = pimc::SingleValueMoveAdjuster<double> {range, abs_adjustment, upper_limits};
        SECTION("not bounded from below")
        {
            const auto current = double {1.05};
            const auto expected = double {0.95};
            const auto actual = move_adjuster.adjust_step(current, move_tracker_too_low);
            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("bounded from above")
        {
            const auto current = double {4.95};
            const auto expected = double {5.0};
            const auto actual = move_adjuster.adjust_step(current, move_tracker_too_high);
            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }

        SECTION("bounded not applied")
        {
            const auto current = double {2.5};
            const auto expected = double {2.6};
            const auto actual = move_adjuster.adjust_step(current, move_tracker_too_high);
            REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
        }
    }
}
