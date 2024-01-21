#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "coordinates/measure.hpp"
#include "geometries/lattice.hpp"
#include "geometries/unit_cell.hpp"
#include "geometries/unit_cell_translations.hpp"

static constexpr auto operator""_uz(unsigned long long n) -> std::size_t
{
    return std::size_t {n};
}

TEST_CASE("trivial lattice_particle_positions")
{
    using Point = coord::Cartesian<double, 2>;

    // clang-format off
    const auto unit_cell = []() {
        auto basis_lattice_vectors = std::array { Point {1.0, 0.0}, Point {0.0, 1.0} };
        auto basis_unit_cell_sites = std::vector { Point {0.0, 0.0} };
        return geom::UnitCell<double, 2> {std::move(basis_lattice_vectors), std::move(basis_unit_cell_sites)};
    }();
    // clang-format on

    const auto translations = geom::UnitCellTranslations<2> {2_uz, 3_uz};

    const auto positions = geom::lattice_particle_positions(unit_cell, translations);

    REQUIRE(positions.size() == 6);
    REQUIRE(coord::approx_eq(positions[0], Point {0.0, 0.0}));
    REQUIRE(coord::approx_eq(positions[1], Point {1.0, 0.0}));
    REQUIRE(coord::approx_eq(positions[2], Point {0.0, 1.0}));
    REQUIRE(coord::approx_eq(positions[3], Point {1.0, 1.0}));
    REQUIRE(coord::approx_eq(positions[4], Point {0.0, 2.0}));
    REQUIRE(coord::approx_eq(positions[5], Point {1.0, 2.0}));
}
