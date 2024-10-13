#include <iterator>
#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "coordinates/measure.hpp"
#include "worldline/worldline.hpp"
#include "worldline/writers/read_worldlines.hpp"

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

TEST_CASE("basic read worldlines", "[Worldline]")
{
    auto stream = std::stringstream {};
    stream << "# This file contains the positions of all the beads in all the particles in a simulation\n";
    stream << "# The information after the comments is laid out in the following manner:               \n";
    stream << "# - [integer] block index of the simulation this snapshot is taken at                   \n";
    stream << "# - [integer] NDIM: number of dimensions the simulation was performed in                \n";
    stream << "# - [integer] n_particles: total number of particles                                    \n";
    stream << "# - [integer] n_timeslices: total number of timeslices                                  \n";
    stream << "# ... followed by the bead positions...                                                 \n";
    stream << "#                                                                                       \n";
    stream << "# The positions of the beads are laid out in `NDIM` space-separated columns;            \n";
    stream << "#   - the first `n_particle` lines correspond to the 0th worldline                      \n";
    stream << "#   - the next `n_particle` lines correspond to the 1st worldline                       \n";
    stream << "#   - the next `n_particle` lines correspond to the 2nd worldline, and so on            \n";
    stream << "#   - there are `n_timeslices` worldlines in total                                      \n";
    stream << "10                                                                                      \n";
    stream << "3                                                                                       \n";
    stream << "4                                                                                       \n";
    stream << "2                                                                                       \n";
    stream << "-2.43531882e-01   -1.82242452e-01    2.46618159e-01                                     \n";
    stream << "-1.38601913e-01    3.30594232e+00   -1.91322270e-01                                     \n";
    stream << " 1.82466528e+00    7.24260147e-01    3.19777664e+00                                     \n";
    stream << "-7.06441454e-01    4.01323907e+00    3.30114762e+00                                     \n";
    stream << " 3.97260972e+00   -1.85454391e-01    3.76458239e-01                                     \n";
    stream << " 3.86765307e+00    3.51418714e+00    4.61786052e-02                                     \n";
    stream << " 4.76010234e+00    1.62519369e+00    3.27951001e+00                                     \n";
    stream << " 2.55859115e+00    4.46821617e+00    3.44368619e+00                                     \n";

    const auto worldlines = worldline::read_worldlines<double, 3>(stream);

    REQUIRE(worldlines.n_worldlines() == 4);
    REQUIRE(worldlines.n_timeslices() == 2);

    const auto point00 = coord::Cartesian<double, 3> {-2.43531882e-01, -1.82242452e-01, 2.46618159e-01};
    const auto point01 = coord::Cartesian<double, 3> {-1.38601913e-01, 3.30594232e+00, -1.91322270e-01};
    const auto point02 = coord::Cartesian<double, 3> {1.82466528e+00, 7.24260147e-01, 3.19777664e+00};
    const auto point03 = coord::Cartesian<double, 3> {-7.06441454e-01, 4.01323907e+00, 3.30114762e+00};
    const auto point10 = coord::Cartesian<double, 3> {3.97260972e+00, -1.85454391e-01, 3.76458239e-01};
    const auto point11 = coord::Cartesian<double, 3> {3.86765307e+00, 3.51418714e+00, 4.61786052e-02};
    const auto point12 = coord::Cartesian<double, 3> {4.76010234e+00, 1.62519369e+00, 3.27951001e+00};
    const auto point13 = coord::Cartesian<double, 3> {2.55859115e+00, 4.46821617e+00, 3.44368619e+00};

    REQUIRE(coord::approx_eq(worldlines.get(0, 0), point00));
    REQUIRE(coord::approx_eq(worldlines.get(0, 1), point01));
    REQUIRE(coord::approx_eq(worldlines.get(0, 2), point02));
    REQUIRE(coord::approx_eq(worldlines.get(0, 3), point03));
    REQUIRE(coord::approx_eq(worldlines.get(1, 0), point10));
    REQUIRE(coord::approx_eq(worldlines.get(1, 1), point11));
    REQUIRE(coord::approx_eq(worldlines.get(1, 2), point12));
    REQUIRE(coord::approx_eq(worldlines.get(1, 3), point13));
}
