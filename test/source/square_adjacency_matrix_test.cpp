#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "mathtools/grid/square_adjacency_matrix.hpp"

TEST_CASE("basic SquareAdjacencyMatrix test")
{
    auto adjmat = mathtools::SquareAdjacencyMatrix {5};

    SECTION("adding elements")
    {
        adjmat.add_neighbour(0, 2);
        adjmat.add_neighbour(0, 3);
        adjmat.add_neighbour(0, 4);

        const auto expected = std::vector<std::size_t> {2, 3, 4};

        // create the vector of neighbours from the span
        auto actual = std::vector<std::size_t> {};
        for (auto neigh : adjmat.neighbours(0)) {
            actual.push_back(neigh);
        }

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
}
