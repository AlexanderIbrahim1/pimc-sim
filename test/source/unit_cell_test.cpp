#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "geometries/unit_cell.hpp"

TEST_CASE("unit cell construction", "[UnitCell]")
{
    using Point = coord::Cartesian<double, 3>;

    // NOTE: this weird indentation is because of the alias; writing out the type in its entirety
    // makes this weird extra indentation go away
    const auto lattice_vectors = std::array<Point, 3> {
        Point {1.0, 0.0, 0.0},
         Point {0.0, 1.0, 0.0},
         Point {0.0, 0.0, 1.0}
    };
    const auto unit_cell_sites = std::vector<Point> {
        Point {1.0, 2.0, 3.0}
    };

    const auto unit_cell = geom::UnitCell<double, 3> {lattice_vectors, unit_cell_sites};

    REQUIRE(coord::approx_eq_containers(unit_cell.basis_lattice_vectors(), lattice_vectors));
    REQUIRE(coord::approx_eq_containers(unit_cell.basis_unit_cell_sites(), unit_cell_sites));
    REQUIRE(unit_cell.n_unit_cell_sites() == unit_cell_sites.size());
}

TEST_CASE("unit cell construction : throwing", "[UnitCell]")
{
    using Point = coord::Cartesian<double, 2>;
    using UnitCell = geom::UnitCell<double, 2>;
    // NOTE: can't use template instantiations inside of REQUIRE_THROWS_AS?
    // - so I need to create the type alias for UnitCell

    SECTION("zero unit cell sites")
    {
        const auto lattice_vectors = std::array<Point, 2> {
            Point {1.0, 0.0},
             Point {0.0, 1.0}
        };

        const auto unit_cell_sites = std::vector<Point> {};

        REQUIRE_THROWS_AS(UnitCell(lattice_vectors, unit_cell_sites), std::runtime_error);
    }

    SECTION("lattice basis vector too small")
    {
        const auto lattice_vectors = std::array<Point, 2> {
            Point {0.0, 0.0},
             Point {0.0, 1.0}
        };

        const auto unit_cell_sites = std::vector<Point> {
            Point {0.0, 0.0}
        };

        REQUIRE_THROWS_AS(UnitCell(lattice_vectors, unit_cell_sites), std::runtime_error);
    }
}

TEST_CASE("unit cell sites", "[UnitCell]")
{
    using Point = coord::Cartesian<double, 2>;
    using UnitCell = geom::UnitCell<double, 2>;

    SECTION("orthogonal elementary 2D")
    {
        const auto basis_lattice_vectors = std::array<Point, 2> {
            Point {1.0, 0.0},
             Point {0.0, 1.0}
        };
        const auto basis_unit_cell_sites = std::vector<Point> {
            Point {0.0, 0.0},
             Point {0.5, 0.5}
        };
        const auto unit_cell = UnitCell {basis_lattice_vectors, basis_unit_cell_sites};

        SECTION("lattice point at origin")
        {
            const auto unit_cell_sites = geom::unit_cell_sites(unit_cell, Point {0.0, 0.0});
            REQUIRE(coord::approx_eq(unit_cell_sites[0], Point {0.0, 0.0}));
            REQUIRE(coord::approx_eq(unit_cell_sites[1], Point {0.5, 0.5}));
        }

        SECTION("lattice point not at origin")
        {
            const auto unit_cell_sites = geom::unit_cell_sites(unit_cell, Point {0.25, 0.25});
            REQUIRE(coord::approx_eq(unit_cell_sites[0], Point {0.25, 0.25}));
            REQUIRE(coord::approx_eq(unit_cell_sites[1], Point {0.75, 0.75}));
        }
    }
}
