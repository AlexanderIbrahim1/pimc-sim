#include <array>
#include <initializer_list>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "coordinates/measure.hpp"
#include "worldline/worldline.hpp"
#include "worldline/writers/worldline_writer.hpp"

TEST_CASE("worldline construction", "[Worldline]")
{
    using Point = coord::Cartesian<double, 3>;

    const auto point0 = Point {1.0, 2.0, 3.0};
    const auto point1 = Point {4.0, 5.0, 6.0};
    const auto point2 = Point {7.0, 8.0, 9.0};

    SECTION("from array")
    {
        const auto arr = std::array<Point, 3> {point0, point1, point2};
        const auto wline = worldline::Worldline<double, 3> {arr};

        const auto points = wline.points();
        REQUIRE(coord::approx_eq(point0, points[0]));
        REQUIRE(coord::approx_eq(point1, points[1]));
        REQUIRE(coord::approx_eq(point2, points[2]));
    }

    SECTION("from vector")
    {
        const auto vec = std::vector<Point> {point0, point1, point2};
        const auto wline = worldline::Worldline<double, 3> {vec};

        const auto points = wline.points();
        REQUIRE(coord::approx_eq(point0, points[0]));
        REQUIRE(coord::approx_eq(point1, points[1]));
        REQUIRE(coord::approx_eq(point2, points[2]));
    }

    SECTION("from initializer list")
    {
        const auto ilist = std::initializer_list<Point> {point0, point1, point2};
        const auto wline = worldline::Worldline<double, 3> {ilist};

        const auto points = wline.points();
        REQUIRE(coord::approx_eq(point0, points[0]));
        REQUIRE(coord::approx_eq(point1, points[1]));
        REQUIRE(coord::approx_eq(point2, points[2]));
    }

    SECTION("with initializer list")
    {
        const auto wline = worldline::Worldline<double, 3> {point0, point1, point2};

        const auto points = wline.points();
        REQUIRE(coord::approx_eq(point0, points[0]));
        REQUIRE(coord::approx_eq(point1, points[1]));
        REQUIRE(coord::approx_eq(point2, points[2]));
    }
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

    REQUIRE(worldlines.size() == 2);
    REQUIRE(worldlines[0].size() == 4);
    REQUIRE(worldlines[1].size() == 4);

    const auto point00 = coord::Cartesian<double, 3> {-2.43531882e-01, -1.82242452e-01, 2.46618159e-01};
    const auto point01 = coord::Cartesian<double, 3> {-1.38601913e-01, 3.30594232e+00, -1.91322270e-01};
    const auto point02 = coord::Cartesian<double, 3> {1.82466528e+00, 7.24260147e-01, 3.19777664e+00};
    const auto point03 = coord::Cartesian<double, 3> {-7.06441454e-01, 4.01323907e+00, 3.30114762e+00};
    const auto point10 = coord::Cartesian<double, 3> {3.97260972e+00, -1.85454391e-01, 3.76458239e-01};
    const auto point11 = coord::Cartesian<double, 3> {3.86765307e+00, 3.51418714e+00, 4.61786052e-02};
    const auto point12 = coord::Cartesian<double, 3> {4.76010234e+00, 1.62519369e+00, 3.27951001e+00};
    const auto point13 = coord::Cartesian<double, 3> {2.55859115e+00, 4.46821617e+00, 3.44368619e+00};

    REQUIRE(coord::approx_eq(worldlines[0][0], point00));
    REQUIRE(coord::approx_eq(worldlines[0][1], point01));
    REQUIRE(coord::approx_eq(worldlines[0][2], point02));
    REQUIRE(coord::approx_eq(worldlines[0][3], point03));
    REQUIRE(coord::approx_eq(worldlines[1][0], point10));
    REQUIRE(coord::approx_eq(worldlines[1][1], point11));
    REQUIRE(coord::approx_eq(worldlines[1][2], point12));
    REQUIRE(coord::approx_eq(worldlines[1][3], point13));
}
