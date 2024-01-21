#pragma once

#include <concepts>
#include <cstddef>

#include <coordinates/cartesian.hpp>
#include <geometries/unit_cell.hpp>
#include <geometries/unit_cell_translations.hpp>

namespace geom
{

enum class LatticeType
{
    HCP
};

template <std::floating_point FP, std::size_t NDIM>
constexpr auto lattice_particle_positions(
    const UnitCell<FP, NDIM>& unit_cell,
    const UnitCellTranslations<NDIM>& translations
) -> std::vector<coord::Cartesian<FP, NDIM>>
{
    const auto n_sites = n_total_boxes(translations);
    const auto basis_size = unit_cell.n_basis_unit_cell_sites();

    auto sites = std::vector<coord::Cartesian<FP, NDIM>> {};
    sites.reserve(n_sites);

    const auto unit_cell_incrementer = UnitCellIncrementer<NDIM> {translations};

    for (std::size_t i_site {0}; i_size < n_sites; ++i_site) {
    }
}

}  // namespace geom
