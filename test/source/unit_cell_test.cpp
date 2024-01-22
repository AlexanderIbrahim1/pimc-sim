#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
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
    REQUIRE(unit_cell.n_basis_unit_cell_sites() == unit_cell_sites.size());
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

TEST_CASE("orthogonal and elementary", "[UnitCell]")
{
    SECTION("is orthogonal and elementary : 2D")
    {
        using Point = coord::Cartesian<double, 2>;
        const auto basis_lattice_vectors = std::array {
            Point {1.0, 0.0},
             Point {0.0, 1.0}
        };
        REQUIRE(geom::is_orthogonal_and_elementary(basis_lattice_vectors));
    }

    SECTION("is orthogonal and elementary : 3D")
    {
        using Point = coord::Cartesian<double, 3>;
        const auto basis_lattice_vectors = std::array {
            Point {0.0, 2.0, 0.0},
             Point {1.0, 0.0, 0.0},
             Point {0.0, 0.0, 3.0}
        };
        REQUIRE(geom::is_orthogonal_and_elementary(basis_lattice_vectors));
    }

    SECTION("is not orthogonal and elementary : 2D")
    {
        using Point = coord::Cartesian<double, 2>;
        const auto basis_lattice_vectors = std::array {
            Point {1.0, 0.4},
             Point {0.0, 1.0}
        };
        REQUIRE(!geom::is_orthogonal_and_elementary(basis_lattice_vectors));
    }

    SECTION("is orthogonal and elementary : 3D")
    {
        using Point = coord::Cartesian<double, 3>;
        const auto basis_lattice_vectors = std::array {
            Point {1.0, 2.0, 3.0},
             Point {4.0, 5.0, 6.0},
             Point {7.0, 8.0, 9.0}
        };
        REQUIRE(!geom::is_orthogonal_and_elementary(basis_lattice_vectors));
    }
}

TEST_CASE("unit_cell_box_sides", "[UnitCell]")
{
    SECTION("2-dimensional lattice cell")
    {
        using Point = coord::Cartesian<double, 2>;
        using Box = coord::BoxSides<double, 2>;

        // clang-format off
        const auto basis_lattice_vectors = GENERATE_COPY(
            std::array {Point {1.0, 0.0}, Point {0.0, 2.0}},
            std::array {Point {-1.0, 0.0}, Point {0.0, 2.0}},
            std::array {Point {1.0, 0.0}, Point {0.0, -2.0}},
            std::array {Point {-1.0, 0.0}, Point {0.0, -2.0}}
        );
        const auto basis_unit_cell_sites = std::vector {Point {0.0, 0.0}};
        // clang-format on

        const auto unit_cell = geom::UnitCell<double, 2> {basis_lattice_vectors, basis_unit_cell_sites};

        const auto expected_box = Box {1.0, 2.0};
        const auto actual_box = geom::unit_cell_box_sides(unit_cell);

        REQUIRE(coord::approx_eq(expected_box, actual_box));
    }

    SECTION("throw if unit cell is not orthogonal and elementary")
    {
        using Point = coord::Cartesian<double, 2>;

        // clang-format off
        const auto basis_lattice_vectors = std::array {Point {1.0, 0.5}, Point {0.0, 2.0}};
        const auto basis_unit_cell_sites = std::vector {Point {0.0, 0.0}};
        const auto unit_cell = geom::UnitCell<double, 2> {basis_lattice_vectors, basis_unit_cell_sites};
        // clang-format on

        REQUIRE_THROWS_AS(geom::unit_cell_box_sides(unit_cell), std::runtime_error);
    }
}
