#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <format>
#include <utility>
#include <vector>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <geometries/constants.hpp>

namespace geom
{

// TODO: add a check to make sure the lattice site lies inside the unit cell
//      - this was done in the Python version using some matrix trick
// TODO: add a function that checks if a unit cell is orthogonal and elementary

template <std::floating_point FP, std::size_t NDIM>
class UnitCell
{
    using Point = coord::Cartesian<FP, NDIM>;

public:
    constexpr explicit UnitCell(
        std::array<Point, NDIM> lattice_vectors,
        std::vector<Point> unit_cell_sites
    )
        : basis_lattice_vectors_ {std::move(lattice_vectors)}
        , basis_unit_cell_sites_ {std::move(unit_cell_sites)}
    {
        if (basis_unit_cell_sites_.size() < 1) {
            throw std::runtime_error(
                "There must be at least one lattice site per conventional unit cell."
            );
        }

        for (const auto& lvec : basis_lattice_vectors_) {
            if (coord::norm_squared(lvec) < EPSILON_MINIMUM_LATTICE_VECTOR_NORM_SQUARED<FP>) {
                throw std::runtime_error(std::format(
                    "All lattice vectors must have a non-zero length.\nFound: {}", lvec.as_string()
                ));
            }
        }
    }

    constexpr auto basis_lattice_vectors() const noexcept -> const std::array<Point, NDIM>&
    {
        return basis_lattice_vectors_;
    }

    constexpr auto basis_unit_cell_sites() const noexcept -> const std::vector<Point>&
    {
        return basis_unit_cell_sites_;
    }

    constexpr auto n_unit_cell_sites() const noexcept -> std::size_t
    {
        return basis_unit_cell_sites_.size();
    }

private:
    std::array<Point, NDIM> basis_lattice_vectors_;
    std::vector<Point> basis_unit_cell_sites_;
};

template <std::floating_point FP, std::size_t NDIM>
constexpr auto unit_cell_sites(
    const UnitCell<FP, NDIM>& unit_cell,
    const coord::Cartesian<FP, NDIM>& lattice_point
) -> std::vector<coord::Cartesian<FP, NDIM>>
{
    auto sites = std::vector<coord::Cartesian<FP, NDIM>> {};
    sites.reserve(unit_cell.n_unit_cell_sites());
    for (const auto& lvec : unit_cell.basis_unit_cell_sites()) {
        sites.emplace_back(lattice_point + lvec);
    }

    return sites;
}

}  // namespace geom
