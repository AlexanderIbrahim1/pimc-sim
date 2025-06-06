#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "coordinates/box_sides.hpp"
#include "coordinates/cartesian.hpp"
#include "coordinates/measure_wrappers.hpp"
#include "interactions/handlers/nearest_neighbour_interaction_handler.hpp"
#include "mathtools/grid/square_adjacency_matrix.hpp"
#include "worldline/worldline.hpp"

constexpr auto collect_neighbours(const mathtools::SquareAdjacencyMatrix& adjmat, std::size_t i_source) noexcept
{
    auto targets = std::vector<std::size_t> {};
    for (auto neigh : adjmat.neighbours(i_source)) {
        targets.push_back(neigh);
    }

    return targets;
}

TEST_CASE("basic SquareAdjacencyMatrix test")
{
    auto adjmat = mathtools::SquareAdjacencyMatrix {5};

    SECTION("adding elements")
    {
        adjmat.add_neighbour(0, 2);
        adjmat.add_neighbour(0, 3);
        adjmat.add_neighbour(0, 4);

        const auto expected = std::vector<std::size_t> {2, 3, 4};
        const auto actual = collect_neighbours(adjmat, 0);

        REQUIRE(expected == actual);
    }

    SECTION("clearing")
    {
        adjmat.add_neighbour(0, 2);
        adjmat.add_neighbour(0, 3);
        adjmat.add_neighbour(0, 4);

        REQUIRE(adjmat.neighbours(0).size() == 3);

        adjmat.clear(0);

        REQUIRE(adjmat.neighbours(0).size() == 0);
    }

    SECTION("add both at once")
    {
        adjmat.add_neighbour_both(0, 1);
        adjmat.add_neighbour_both(0, 2);
        adjmat.add_neighbour_both(0, 3);
        adjmat.add_neighbour_both(3, 1);

        REQUIRE(collect_neighbours(adjmat, 0) == std::vector<std::size_t> {1, 2, 3});
        REQUIRE(collect_neighbours(adjmat, 1) == std::vector<std::size_t> {0, 3});
        REQUIRE(collect_neighbours(adjmat, 2) == std::vector<std::size_t> {0});
        REQUIRE(collect_neighbours(adjmat, 3) == std::vector<std::size_t> {0, 1});
    }
}

TEST_CASE("update adjacency matrix")
{
    using Point = coord::Cartesian<double, 2>;

    const auto box = coord::BoxSides<double, 2> {1.0, 1.0};
    const auto cutoff_distance = double {0.25};
    const auto n_timeslices = std::size_t {8};
    const auto n_particles = std::size_t {5};

    auto adjmat = mathtools::SquareAdjacencyMatrix {n_particles};

    const auto worldlines = [&]()
    {
        const auto positions = std::vector<Point> {
            Point {0.0,  0.0},
             Point {0.1,  0.0},
             Point {-0.1, 0.0},
             Point {0.0,  0.4},
             Point {0.3,  0.0}
        };

        auto wlines = worldline::Worldlines<double, 2> {n_timeslices, n_particles};
        for (std::size_t it {0}; it < n_timeslices; ++it) {
            for (std::size_t ip {0}; ip < n_particles; ++ip) {
                wlines.set(it, ip, positions[ip]);
            }
        }

        return wlines;
    }();

    REQUIRE(adjmat.neighbours(0).size() == 0);
    REQUIRE(adjmat.neighbours(1).size() == 0);
    REQUIRE(adjmat.neighbours(2).size() == 0);
    REQUIRE(adjmat.neighbours(3).size() == 0);
    REQUIRE(adjmat.neighbours(4).size() == 0);

    const auto periodic_dist_sq_calculator = coord::PeriodicDistanceSquaredMeasureWrapper<double, 2> {box};
    interact::update_centroid_adjacency_matrix<double, 2>(
        worldlines, periodic_dist_sq_calculator, adjmat, cutoff_distance
    );

    REQUIRE(collect_neighbours(adjmat, 0) == std::vector<std::size_t> {1, 2});
    REQUIRE(collect_neighbours(adjmat, 1) == std::vector<std::size_t> {0, 2, 4});
    REQUIRE(collect_neighbours(adjmat, 2) == std::vector<std::size_t> {0, 1});
    REQUIRE(collect_neighbours(adjmat, 3).size() == 0);
    REQUIRE(collect_neighbours(adjmat, 4) == std::vector<std::size_t> {1});
}
