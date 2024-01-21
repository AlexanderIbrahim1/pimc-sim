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
constexpr auto lattice_sites(
    const UnitCell<FP, NDIM>& unit_cell,
    const UnitCellTranslations<NDIM>& translations
) -> std::vector<coord::Cartesian<FP, NDIM>>
{
    auto sites = std::vector<coord::Cartesian<FP, NDIM>> {};

    const auto n_sites = n_total_boxes(translations);
    sites.reserve(n_sites);
}

}  // namespace geom
