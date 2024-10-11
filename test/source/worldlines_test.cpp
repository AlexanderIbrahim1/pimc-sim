#include <iterator>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "coordinates/measure.hpp"
#include "worldline/worldline.hpp"

auto get_worldlines_2d_2timeslices_4particles() -> worldline::Worldlines<double, 2>
{
    using Point = coord::Cartesian<double, 2>;

    auto worldlines = worldline::Worldlines<double, 2> {2, 4};
    worldlines.set(0, 0, Point {0.0, 0.0});
    worldlines.set(0, 1, Point {0.0, 1.0});
    worldlines.set(0, 2, Point {0.0, 2.0});
    worldlines.set(0, 3, Point {0.0, 3.0});
    worldlines.set(1, 0, Point {1.0, 0.0});
    worldlines.set(1, 1, Point {1.0, 1.0});
    worldlines.set(1, 2, Point {1.0, 2.0});
    worldlines.set(1, 3, Point {1.0, 3.0});

    return worldlines;
}

TEST_CASE("Worldlines: basic test")
{
    using Point = coord::Cartesian<double, 3>;

    // 2 particles (worldlines), 3 imaginary time steps
    auto worldlines = worldline::Worldlines<double, 3> {3, 2};

    REQUIRE(worldlines.n_timeslices() == 3);
    REQUIRE(worldlines.n_worldlines() == 2);

    worldlines.set(0, 0, Point {1.1, 2.2, 3.3});
    worldlines.set(1, 0, Point {4.4, 5.5, 6.6});
    worldlines.set(2, 0, Point {7.7, 8.8, 9.9});
    worldlines.set(0, 1, Point {10.1, 20.2, 30.3});
    worldlines.set(1, 1, Point {40.4, 50.5, 60.6});
    worldlines.set(2, 1, Point {70.7, 80.8, 90.9});

    REQUIRE(coord::approx_eq(worldlines.get(0, 0), Point {1.1, 2.2, 3.3}));
    REQUIRE(coord::approx_eq(worldlines.get(1, 0), Point {4.4, 5.5, 6.6}));
    REQUIRE(coord::approx_eq(worldlines.get(2, 0), Point {7.7, 8.8, 9.9}));
    REQUIRE(coord::approx_eq(worldlines.get(0, 1), Point {10.1, 20.2, 30.3}));
    REQUIRE(coord::approx_eq(worldlines.get(1, 1), Point {40.4, 50.5, 60.6}));
    REQUIRE(coord::approx_eq(worldlines.get(2, 1), Point {70.7, 80.8, 90.9}));
}

TEST_CASE("Worldlines: iterator")
{
    using Point = coord::Cartesian<double, 2>;

    const auto worldlines = get_worldlines_2d_2timeslices_4particles();

    SECTION("along timeslices")
    {
        SECTION("timeslice 0")
        {
            const auto expected = std::vector<Point> {
                Point {0.0, 0.0},
                 Point {0.0, 1.0},
                 Point {0.0, 2.0},
                 Point {0.0, 3.0}
            };

            auto actual = std::vector<Point> {};
            for (const auto& point : worldlines.timeslice(0)) {
                actual.push_back(point);
            }

            REQUIRE(coord::approx_eq_containers<double, 2>(expected, actual));
        }

        SECTION("timeslice 1")
        {
            const auto expected = std::vector<Point> {
                Point {1.0, 0.0},
                 Point {1.0, 1.0},
                 Point {1.0, 2.0},
                 Point {1.0, 3.0}
            };

            auto actual = std::vector<Point> {};
            for (const auto& point : worldlines.timeslice(1)) {
                actual.push_back(point);
            }

            REQUIRE(coord::approx_eq_containers<double, 2>(expected, actual));
        }
    }

    SECTION("along worldlines")
    {
        SECTION("worldline 0")
        {
            const auto expected = std::vector<Point> {
                Point {0.0, 0.0},
                 Point {1.0, 0.0}
            };

            const auto iter = worldlines.worldline(0);
            const auto actual = std::vector<Point> {iter.begin(), iter.end()};

            REQUIRE(actual.size() == 2);
            REQUIRE(coord::approx_eq_containers<double, 2>(expected, actual));
        }

        SECTION("worldline 1")
        {
            const auto expected = std::vector<Point> {
                Point {0.0, 1.0},
                 Point {1.0, 1.0}
            };

            const auto iter = worldlines.worldline(1);
            const auto actual = std::vector<Point> {iter.begin(), iter.end()};

            REQUIRE(actual.size() == 2);
            REQUIRE(coord::approx_eq_containers<double, 2>(expected, actual));
        }
    }
}

TEST_CASE("Worldlines: beads on timeslice are contiguous")
{
    const auto worldlines = get_worldlines_2d_2timeslices_4particles();

    const auto& bead00 = worldlines.get(0, 0);
    const auto& bead01 = worldlines.get(0, 1);
    const auto& bead02 = worldlines.get(0, 2);
    const auto& bead03 = worldlines.get(0, 3);
    const auto& bead10 = worldlines.get(1, 0);

    const auto distance_01_00 = &bead01 - &bead00;
    const auto distance_02_01 = &bead02 - &bead01;
    const auto distance_03_02 = &bead03 - &bead02;
    const auto distance_10_00 = &bead10 - &bead00;

    REQUIRE(distance_01_00 == 1);
    REQUIRE(distance_02_01 == 1);
    REQUIRE(distance_03_02 == 1);
    REQUIRE(distance_10_00 == 4);
}
