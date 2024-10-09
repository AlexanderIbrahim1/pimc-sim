#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <geometries/constants.hpp>

namespace geom
{

// TODO: add a check to make sure the lattice site lies inside the unit cell
//      - this was done in the Python version using some matrix trick

template <std::floating_point FP, std::size_t NDIM>
class UnitCell
{
    using Point = coord::Cartesian<FP, NDIM>;

public:
    constexpr explicit UnitCell(std::array<Point, NDIM> lattice_vectors, std::vector<Point> unit_cell_sites)
        : basis_lattice_vectors_ {std::move(lattice_vectors)}
        , basis_unit_cell_sites_ {std::move(unit_cell_sites)}
    {
        if (basis_unit_cell_sites_.size() < 1) {
            throw std::runtime_error("There must be at least one lattice site per conventional unit cell.");
        }

        for (const auto& lvec : basis_lattice_vectors_) {
            if (coord::norm_squared(lvec) < EPSILON_MINIMUM_LATTICE_VECTOR_NORM_SQUARED<FP>) {
                throw std::runtime_error {
                    "All lattice vectors must have a non-zero length when constructing a UnitCell<FP, NDIM>."};
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

    constexpr auto n_basis_unit_cell_sites() const noexcept -> std::size_t
    {
        return basis_unit_cell_sites_.size();
    }

private:
    std::array<Point, NDIM> basis_lattice_vectors_;
    std::vector<Point> basis_unit_cell_sites_;
};

template <std::floating_point FP, std::size_t NDIM>
constexpr auto unit_cell_sites(const UnitCell<FP, NDIM>& unit_cell, const coord::Cartesian<FP, NDIM>& lattice_point)
    -> std::vector<coord::Cartesian<FP, NDIM>>
{
    auto sites = std::vector<coord::Cartesian<FP, NDIM>> {};
    sites.reserve(unit_cell.n_basis_unit_cell_sites());
    for (const auto& lvec : unit_cell.basis_unit_cell_sites()) {
        sites.emplace_back(lattice_point + lvec);
    }

    return sites;
}

struct NonzeroResult
{
    bool is_valid {};
    std::size_t index {};
};

template <std::floating_point FP, std::size_t NDIM>
constexpr auto find_unique_nonzero_index(const coord::Cartesian<FP, NDIM>& point) -> NonzeroResult
{
    const auto is_nonzero = [](FP x) { return std::fabs(x) >= EPSILON_MINIMUM_COORDINATE_ABSOLUTE_VALUE<FP>; };

    auto n_nonzeroes = std::size_t {0};
    auto i_nonzero = std::size_t {0};

    for (std::size_t i {0}; i < NDIM; ++i) {
        if (is_nonzero(point[i])) {
            i_nonzero = i;
            ++n_nonzeroes;
        }
    }

    if (n_nonzeroes != 1) {
        return {false, 0};
    }
    else {
        return {true, i_nonzero};
    }
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto is_orthogonal_and_elementary(const std::array<coord::Cartesian<FP, NDIM>, NDIM>& basis_lattice_vectors
) noexcept -> bool
{
    auto nonzero_flags = std::array<bool, NDIM> {};

    for (const auto& lvec : basis_lattice_vectors) {
        const auto result = find_unique_nonzero_index(lvec);
        if (result.is_valid && !nonzero_flags[result.index]) {
            nonzero_flags[result.index] = true;
        }
        else {
            return false;
        }
    }

    return std::all_of(std::begin(nonzero_flags), std::end(nonzero_flags), [](bool flag) { return flag; });
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto unit_cell_box_sides(const UnitCell<FP, NDIM>& unit_cell) -> coord::BoxSides<FP, NDIM>
{
    const auto basis = unit_cell.basis_lattice_vectors();
    if (!is_orthogonal_and_elementary(basis)) {
        throw std::runtime_error(
            "Right now, we can only get unit cell box sides for unit cells whose basis lattice vectors are\n"
            "orthogonal and elementary.\n"
        );
    }

    auto unit_cell_sides = coord::Cartesian<FP, NDIM>::origin();
    for (const auto& bvec : basis) {
        unit_cell_sides += bvec;
    }

    // the basis lattice vectors aren't guaranteed to all point in the positive cardinal directions, but
    // the box (whose entries are defined as lengths) must all be positive
    for (std::size_t i {0}; i < NDIM; ++i) {
        unit_cell_sides[i] = std::fabs(unit_cell_sides[i]);
    }

    return coord::BoxSides<FP, NDIM> {unit_cell_sides};
}

}  // namespace geom
