#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <utility>
#include <vector>

#include <coordinates/cartesian.hpp>

#include <geometries/geom_utils.hpp>
#include <geometries/unit_cell.hpp>

namespace geom
{

template <std::floating_point FP>
auto conventional_hcp_unit_cell(FP lattice_constant) -> UnitCell<FP, 3>
{
    using Point = coord::Cartesian<FP, 3>;

    geom_utils::check_lattice_constant_is_positive(lattice_constant);

    const auto lat_const_x = lattice_constant;
    const auto lat_const_y = std::sqrt(3.0) * lattice_constant;
    const auto lat_const_z = std::sqrt(8.0 / 3.0) * lattice_constant;

    auto basis_lattice_vectors = std::array {
        Point {lat_const_x, 0.000000000, 0.000000000},
        Point {0.000000000, lat_const_y, 0.000000000},
        Point {0.000000000, 0.000000000, lat_const_z}
    };

    auto basis_unit_cell_sites = std::vector {
        lattice_constant* Point {0.0, 0.0,                   0.0                 },
        lattice_constant* Point {0.5, std::sqrt(3.0 / 4.0),  0.0                 },
        lattice_constant* Point {0.5, std::sqrt(1.0 / 12.0), std::sqrt(2.0 / 3.0)},
        lattice_constant* Point {
                                 0.0, std::sqrt(4.0 / 3.0),  std::sqrt(2.0 / 3.0)}  // x-value can be 0.0 or 1.0, both work
    };

    return UnitCell<FP, 3> {std::move(basis_lattice_vectors), std::move(basis_unit_cell_sites)};
}

}  // namespace geom
