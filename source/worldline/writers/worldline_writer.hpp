#pragma once

#include <concepts>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <coordinates/box_sides.hpp>
#include <worldline/worldline.hpp>

/*
TODO:
- figure out how we can format the file names for the position files
  - just force the user to accept that they can only modify it using the block index?
  - allow the user to provide a prefix?
*/

namespace worldline
{

namespace writer_utils
{

constexpr auto POSITION_SPACING = std::string_view {"   "};

}  // namespace writer_utils

}  // namespace worldline

namespace worldline
{

template <std::floating_point FP, std::size_t NDIM>
static auto periodic_box_simulation_header(
    std::size_t n_particles,
    std::size_t n_timeslices,
    std::size_t i_block,
    coord::BoxSides<FP, NDIM> box
) noexcept -> std::string
{
    auto header = std::stringstream {};
    header << "# This file contains the positions of all the beads in all the particles\n";
    header << "# in a simulation with periodic boundary conditions;\n";
    header << "# The information after the comments is laid out in the following manner:\n";
    header << "# - [integer] block index of the simulation this snapshot is taken at\n";
    header << "# - [integer] NDIM: number of dimensions the simulation was performed in\n";
    header << "# - [integer] n_particles: total number of particles\n";
    header << "# - [integer] n_timeslices: total number of timeslices\n";
    header << "# - [(NDIM many) floating-point] periodic box edges\n";
    header << "# Then the positions\n";
    header << '\n';
    header << "# The positions of the beads are laid out in `NDIM` space-separated columns;\n";
    header << "#   - the first `n_particle` lines correspond to the 0th worldline\n";
    header << "#   - the next `n_particle` lines correspond to the 1st worldline\n";
    header << "#   - the next `n_particle` lines correspond to the 2nd worldline, and so on\n";
    header << "#   - there are `n_timeslices` worldlines in total\n";
    header << i_block << '\n';
    header << NDIM << '\n';
    header << n_particles << '\n';
    header << n_timeslices << '\n';

    header << std::scientific << std::setprecision(8);

    for (std::size_t i {0}; i < NDIM; ++i) {
        header << box[i] << writer_utils::POSITION_SPACING;
    }
    header << '\n';

    return header.str();
}

template <std::floating_point FP, std::size_t NDIM>
static void write_worldlines(const std::vector<worldline::Worldline<FP, NDIM>>& worldlines, std::string_view header)
{}

}  // namespace worldline
