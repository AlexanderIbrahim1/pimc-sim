#include <cmath>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "coordinates/constants.hpp"
#include "coordinates/measure.hpp"
#include "coordinates/periodicboxsides.hpp"

TEST_CASE("distance_squared : three_dimensional", "[Cartesian3D]")
{
    const auto point0 = coord::Cartesian<double, 3> {0.0, 0.0, 0.0};

    SECTION("point 1.0 in x-direction")
    {
        const auto point1 = coord::Cartesian<double, 3> {1.0, 0.0, 0.0};
        const auto dist_sq = coord::distance_squared(point0, point1);
        REQUIRE_THAT(dist_sq, Catch::Matchers::WithinRel(1.0));
    }

    SECTION("point -1.0 in x-direction")
    {
        const auto point1 = coord::Cartesian<double, 3> {-1.0, 0.0, 0.0};
        const auto dist_sq = coord::distance_squared(point0, point1);
        REQUIRE_THAT(dist_sq, Catch::Matchers::WithinRel(1.0));
    }

    SECTION("point 1.0 in y-direction")
    {
        const auto point1 = coord::Cartesian<double, 3> {0.0, 1.0, 0.0};
        const auto dist_sq = coord::distance_squared(point0, point1);
        REQUIRE_THAT(dist_sq, Catch::Matchers::WithinRel(1.0));
    }

    SECTION("point 1.0 in z-direction")
    {
        const auto point1 = coord::Cartesian<double, 3> {0.0, 0.0, 1.0};
        const auto dist_sq = coord::distance_squared(point0, point1);
        REQUIRE_THAT(dist_sq, Catch::Matchers::WithinRel(1.0));
    }

    SECTION("neither point at the origin")
    {
        const auto pointa = coord::Cartesian<double, 3> {-1.0, 0.0, 0.0};
        const auto pointb = coord::Cartesian<double, 3> {0.0, 1.0, 0.0};
        const auto dist_sq = coord::distance_squared(pointa, pointb);
        const auto expected_distance = 2.0;
        REQUIRE_THAT(dist_sq, Catch::Matchers::WithinRel(expected_distance));
    }
}

TEST_CASE("distance_squared : two_dimensional", "[Cartesian2D]")
{
    const auto point0 = coord::Cartesian<double, 2> {0.0, 0.0};

    SECTION("point 1.0 in x-direction")
    {
        const auto point1 = coord::Cartesian<double, 2> {1.0, 0.0};
        const auto dist_sq = coord::distance_squared(point0, point1);
        REQUIRE_THAT(dist_sq, Catch::Matchers::WithinRel(1.0));
    }

    SECTION("point 1.0 in y-direction")
    {
        const auto point1 = coord::Cartesian<double, 2> {0.0, 1.0};
        const auto dist_sq = coord::distance_squared(point0, point1);
        REQUIRE_THAT(dist_sq, Catch::Matchers::WithinRel(1.0));
    }
}

TEST_CASE("distance_squared : one_dimensional", "[Cartesian1D]")
{
    const auto point0 = coord::Cartesian<double, 1> {0.0};

    SECTION("point 1.0 in x-direction")
    {
        const auto point1 = coord::Cartesian<double, 1> {1.0};
        const auto dist_sq = coord::distance_squared(point0, point1);
        REQUIRE_THAT(dist_sq, Catch::Matchers::WithinRel(1.0));
    }
}

TEST_CASE("distance : three_dimensional", "[Cartesian3D]")
{
    const auto pointa = coord::Cartesian<double, 3> {-1.0, 0.0, 0.0};
    const auto pointb = coord::Cartesian<double, 3> {0.0, 1.0, 0.0};
    const auto actual_distance = coord::distance(pointa, pointb);
    const auto expected_distance = std::sqrt(2.0);
    REQUIRE_THAT(actual_distance, Catch::Matchers::WithinRel(expected_distance));
}

TEST_CASE("distance_squared_periodic : three_dimensional : unit box from origin", "[Cartesian3D]")
{
    const auto box = coord::PeriodicBoxSides<float, 3> {1.0f, 1.0f, 1.0f};
    const auto origin = coord::Cartesian<float, 3> {0.0f, 0.0f, 0.0f};

    SECTION("0.0 vs 0.6")
    {
        const auto point = coord::Cartesian<float, 3> {0.6f, 0.0f, 0.0f};
        const auto actual_dist_sq = coord::distance_squared_periodic(origin, point, box);
        const auto expected_dist_sq = 0.4f * 0.4f;

        REQUIRE_THAT(actual_dist_sq, Catch::Matchers::WithinRel(expected_dist_sq));
    }

    SECTION("0.0 vs -0.6")
    {
        const auto point = coord::Cartesian<float, 3> {-0.6f, 0.0f, 0.0f};
        const auto actual_dist_sq = coord::distance_squared_periodic(origin, point, box);
        const auto expected_dist_sq = 0.4f * 0.4f;

        REQUIRE_THAT(actual_dist_sq, Catch::Matchers::WithinRel(expected_dist_sq));
    }

    SECTION("origin vs point just outside box")
    {
        const auto point = coord::Cartesian<float, 3> {0.6f, 0.6f, -0.6f};
        const auto actual_dist_sq = coord::distance_squared_periodic(origin, point, box);
        const auto expected_dist_sq = 3 * 0.4f * 0.4f;

        REQUIRE_THAT(actual_dist_sq, Catch::Matchers::WithinRel(expected_dist_sq));
    }
}

TEST_CASE("distance_squared_periodic : three_dimensional : unit box no origins", "[Cartesian3D]")
{
    const auto box = coord::PeriodicBoxSides<float, 3> {1.0f, 1.0f, 1.0f};
    const auto point0 = coord::Cartesian<float, 3> {0.3f, 0.0f, 0.0f};
    const auto point1 = coord::Cartesian<float, 3> {-0.4f, 0.0f, 0.0f};
    const auto actual_dist_sq = coord::distance_squared_periodic(point0, point1, box);
    const auto expected_dist_sq = 0.3f * 0.3f;

    REQUIRE_THAT(actual_dist_sq, Catch::Matchers::WithinRel(expected_dist_sq));
}

TEST_CASE("distance_squared_periodic : three_dimensional : non-unit-box from origin", "[Cartesian3D]")
{
    const auto box = coord::PeriodicBoxSides<float, 3> {1.0f, 2.0f, 3.0f};
    const auto origin = coord::Cartesian<float, 3> {0.0f, 0.0f, 0.0f};
    const auto point = coord::Cartesian<float, 3> {0.6f, 1.1f, 0.5f};
    const auto actual_dist_sq = coord::distance_squared_periodic(origin, point, box);
    const auto expected_dist_sq = 0.4f * 0.4f + 0.9f * 0.9f + 0.5f * 0.5f;

    REQUIRE_THAT(actual_dist_sq, Catch::Matchers::WithinRel(expected_dist_sq));
}

TEST_CASE("distance_periodic : three_dimensional : non-unit-box from origin", "[Cartesian3D]")
{
    const auto box = coord::PeriodicBoxSides<float, 3> {1.0f, 2.0f, 3.0f};
    const auto origin = coord::Cartesian<float, 3> {0.0f, 0.0f, 0.0f};
    const auto point = coord::Cartesian<float, 3> {0.6f, 1.1f, 0.5f};
    const auto actual_dist = coord::distance_periodic(origin, point, box);
    const auto expected_dist = std::sqrt(0.4f * 0.4f + 0.9f * 0.9f + 0.5f * 0.5f);

    REQUIRE_THAT(actual_dist, Catch::Matchers::WithinRel(expected_dist));
}

TEST_CASE("norm_squared : three_dimensional", "[Cartesian3D]")
{
    using Cartesian3D = coord::Cartesian<double, 3>;

    struct TestPair
    {
        Cartesian3D point {};
        double norm_squared {};
    };

    auto pairs = GENERATE(
        TestPair(Cartesian3D {1.0, 0.0, 0.0}, 1.0),
        TestPair(Cartesian3D {2.0, 0.0, 0.0}, 4.0),
        TestPair(Cartesian3D {0.0, 1.0, 0.0}, 1.0),
        TestPair(Cartesian3D {0.0, -1.0, 0.0}, 1.0),
        TestPair(Cartesian3D {0.0, 1.0, 1.0}, 2.0),
        TestPair(Cartesian3D {0.0, 1.0, -1.0}, 2.0)
    );

    const auto actual_norm_sq = coord::norm_squared(pairs.point);
    const auto expected_norm_sq = pairs.norm_squared;
    REQUIRE_THAT(actual_norm_sq, Catch::Matchers::WithinRel(expected_norm_sq));
}

TEST_CASE("norm : three_dimensional", "[Cartesian3D]")
{
    using Cartesian3D = coord::Cartesian<double, 3>;

    struct TestPair
    {
        Cartesian3D point {};
        double norm {};
    };

    auto pairs = GENERATE(
        TestPair(Cartesian3D {1.0, 0.0, 0.0}, 1.0),
        TestPair(Cartesian3D {2.0, 0.0, 0.0}, 2.0),
        TestPair(Cartesian3D {0.0, 1.0, 0.0}, 1.0),
        TestPair(Cartesian3D {0.0, -1.0, 0.0}, 1.0),
        TestPair(Cartesian3D {0.0, 1.0, 1.0}, std::sqrt(2.0)),
        TestPair(Cartesian3D {0.0, 1.0, -1.0}, std::sqrt(2.0))
    );

    const auto actual_norm = coord::norm(pairs.point);
    const auto expected_norm = pairs.norm;
    REQUIRE_THAT(actual_norm, Catch::Matchers::WithinRel(expected_norm));
}

TEST_CASE("approx_eq : three_dimensional", "[Cartesian3D]")
{
    using Cartesian3D = coord::Cartesian<double, 3>;

    struct TestPair
    {
        Cartesian3D point0 {};
        Cartesian3D point1 {};
    };

    auto eps = std::sqrt(coord::EPSILON_APPROX_EQ_SEPARATION_SQUARED<double>) / 2.0;

    auto pairs = GENERATE_COPY(
        TestPair(Cartesian3D {1.0, 2.0, 3.0}, Cartesian3D {1.0 / 1.0, 4.0 / 2.0, 9.0 / 3.0}),
        TestPair(Cartesian3D {1.0, 2.0, 3.0}, Cartesian3D {1.0 + eps, 2.0 + eps, 3.0 + eps}),
        TestPair(Cartesian3D {0.0, 0.0, 0.0}, Cartesian3D {eps, eps, eps})
    );

    REQUIRE(coord::approx_eq(pairs.point0, pairs.point1));
}

TEST_CASE("not approx_eq : three_dimensional", "[Cartesian3D]")
{
    using Cartesian3D = coord::Cartesian<double, 3>;

    struct TestPair
    {
        Cartesian3D point0 {};
        Cartesian3D point1 {};
    };

    auto eps = std::sqrt(coord::EPSILON_APPROX_EQ_SEPARATION_SQUARED<double>);

    auto pairs = GENERATE_COPY(
        TestPair(Cartesian3D {1.0, 2.0, 3.0}, Cartesian3D {1.0 + eps, 2.0 + eps, 3.0 + eps}),
        TestPair(Cartesian3D {0.0, 0.0, 0.0}, Cartesian3D {eps, eps, eps}),
        TestPair(Cartesian3D {0.0, 0.0, 0.0}, Cartesian3D {10.0, 20.0, 30.0}),
        TestPair(Cartesian3D {0.0, 0.0, 0.0}, Cartesian3D {-10.0, -20.0, -30.0}),
        TestPair(Cartesian3D {1.0, 2.0, 3.0}, Cartesian3D {-10.0, -20.0, -30.0})
    );

    REQUIRE(!coord::approx_eq(pairs.point0, pairs.point1));
}

TEST_CASE("approx_eq_periodic : three_dimensional", "[Cartesian3D]")
{
    using Cartesian3D = coord::Cartesian<double, 3>;

    struct TestPair
    {
        Cartesian3D point0 {};
        Cartesian3D point1 {};
    };

    auto eps = std::sqrt(coord::EPSILON_APPROX_EQ_SEPARATION_SQUARED<double>) / 2.0;
    const auto box = coord::PeriodicBoxSides<double, 3> {6.0, 7.0, 8.0};
    const auto p = Cartesian3D {1.0, 2.0, 3.0};

    auto pairs = GENERATE_COPY(
        TestPair(p, p),
        TestPair(p, Cartesian3D {p[0] + box[0], p[1] + box[1], p[2] + box[2]}),
        TestPair(p, Cartesian3D {p[0] + box[0] + eps, p[1] + box[1] + eps, p[2] + box[2] + eps}),
        TestPair(p, Cartesian3D {p[0] - box[0], p[1] - box[1], p[2] - box[2]}),
        TestPair(p, Cartesian3D {p[0] - box[0] - eps, p[1] - box[1] - eps, p[2] - box[2] - eps}),
        TestPair(Cartesian3D {0.0, 0.0, 0.0}, Cartesian3D {eps, eps, eps})
    );

    REQUIRE(coord::approx_eq_periodic(pairs.point0, pairs.point1, box));
}

TEST_CASE("not approx_eq_periodic : three_dimensional", "[Cartesian3D]")
{
    using Cartesian3D = coord::Cartesian<double, 3>;

    struct TestPair
    {
        Cartesian3D point0 {};
        Cartesian3D point1 {};
    };

    auto eps = std::sqrt(coord::EPSILON_APPROX_EQ_SEPARATION_SQUARED<double>);
    const auto box = coord::PeriodicBoxSides<double, 3> {6.0, 7.0, 8.0};
    const auto p = Cartesian3D {1.0, 2.0, 3.0};

    auto pairs = GENERATE_COPY(
        TestPair(p, Cartesian3D {p[0] + box[0] + eps, p[1] + box[1] + eps, p[2] + box[2] + eps}),
        TestPair(p, Cartesian3D {p[0] - box[0] - eps, p[1] - box[1] - eps, p[2] - box[2] - eps}),
        TestPair(Cartesian3D {0.0, 0.0, 0.0}, Cartesian3D {eps, eps, eps}),
        TestPair(Cartesian3D {1.0, 2.0, 3.0}, Cartesian3D {4.0, 5.0, 6.0})
    );

    REQUIRE(!coord::approx_eq_periodic(pairs.point0, pairs.point1, box));
}
