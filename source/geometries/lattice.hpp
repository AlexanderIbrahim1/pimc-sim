#pragma once

#include <concepts>
#include <cstddef>

#include <coordinates/box_sides.hpp>
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
constexpr auto make_lattice_site(
    const std::array<std::size_t, NDIM>& indices,
    const std::array<coord::Cartesian<FP, NDIM>, NDIM>& basis_lattice_vectors
) -> coord::Cartesian<FP, NDIM>
{
    auto lattice_site_ = coord::Cartesian<FP, NDIM> {};

    for (std::size_t i {0}; i < NDIM; ++i) {
        lattice_site_ += static_cast<FP>(indices[i]) * basis_lattice_vectors[i];
    }

    return lattice_site_;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto lattice_particle_positions(
    const UnitCell<FP, NDIM>& unit_cell,
    const UnitCellTranslations<NDIM>& translations
) -> std::vector<coord::Cartesian<FP, NDIM>>
{
    const auto n_sites = n_total_boxes(translations);
    const auto n_particles = n_sites * unit_cell.n_basis_unit_cell_sites();

    auto particle_positions = std::vector<coord::Cartesian<FP, NDIM>> {};
    particle_positions.reserve(n_particles);

    auto incrementer = UnitCellIncrementer<NDIM> {translations};

    for (std::size_t i_site {0}; i_site < n_sites; ++i_site) {
        const auto lattice_site = make_lattice_site(incrementer.indices(), unit_cell.basis_lattice_vectors());
        for (const auto& unit_cell_site : unit_cell.basis_unit_cell_sites()) {
            particle_positions.push_back(lattice_site + unit_cell_site);
        }
        incrementer.increment();
    }

    return particle_positions;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto lattice_box(
    const coord::BoxSides<FP, NDIM>& unit_cell_sides,
    const UnitCellTranslations<NDIM>& translations  //
) -> coord::BoxSides<FP, NDIM>
{
    const auto boxes_in_each_dimension = translations.translations();

    auto lat_box = coord::Cartesian<FP, NDIM>::origin();
    for (std::size_t i {0}; i < NDIM; ++i) {
        lat_box[i] = static_cast<FP>(boxes_in_each_dimension[i]) * unit_cell_sides[i];
    }

    return coord::BoxSides<FP, NDIM> {lat_box};
}

}  // namespace geom
