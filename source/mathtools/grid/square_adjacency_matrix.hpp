#pragma once

#include <cstddef>
#include <span>
#include <sstream>
#include <stdexcept>

#include <mathtools/grid/grid2d.hpp>
#include <mathtools/mathtools_utils.hpp>

namespace mathtools
{

class SquareAdjacencyMatrix
{
public:
    constexpr SquareAdjacencyMatrix(std::size_t n_particles)
        : n_particles_ {n_particles}
        , index_grid_ {n_particles, n_particles}
        , sizes_(n_particles, 0)
    {}

    void clear(std::size_t i_part)
    {
        mathtools_utils::check_in_bounds(i_part, n_particles_);

        sizes_[i_part] = 0;
        for (std::size_t i {0}; i < n_particles_; ++i) {
            index_grid_.set(i_part, i, 0);
        }
    }

    void clear_all()
    {
        for (std::size_t i {0}; i < n_particles_; ++i) {
            clear(i);
        }
    }

    void add_neighbour(std::size_t i_source, std::size_t i_target)
    {
        // Add `i_target` to the adjacency list of `i_source`
        check_add_neighbour_(i_source, i_target);

        const auto current_size = sizes_[i_source];
        index_grid_.set(i_source, current_size, i_target);
        ++sizes_[i_source];
    }

    void add_neighbour_both(std::size_t i_source, std::size_t i_target)
    {
        // Add `i_target` and `i_source` to each other's adjacency lists
        check_add_neighbour_(i_source, i_target);

        const auto source_size = sizes_[i_source];
        index_grid_.set(i_source, source_size, i_target);
        ++sizes_[i_source];

        const auto target_size = sizes_[i_target];
        index_grid_.set(i_target, target_size, i_source);
        ++sizes_[i_target];
    }

    constexpr auto neighbours(std::size_t i_source) const noexcept -> std::span<const std::size_t>
    {
        const auto data_begin = std::begin(index_grid_.data());
        const auto start_offset = static_cast<std::ptrdiff_t>(i_source * n_particles_);
        const auto end_offset = static_cast<std::ptrdiff_t>(sizes_[i_source]);
        const auto neighbours_start = std::next(data_begin, start_offset);
        const auto neighbours_end = std::next(neighbours_start, end_offset);

        return {neighbours_start, neighbours_end};
    }

private:
    std::size_t n_particles_;
    Grid2D<std::size_t> index_grid_;
    std::vector<std::size_t> sizes_;

    void check_add_neighbour_(std::size_t i_source, std::size_t i_target)
    {
        // NOTE: updating the adjacency matrix is probably not part of the hot loop;
        // the checks don't hurt much here
        mathtools_utils::check_in_bounds(i_source, n_particles_);
        mathtools_utils::check_in_bounds(i_target, n_particles_);

        if (sizes_[i_source] >= n_particles_) {
            auto err_msg = std::stringstream {};
            err_msg << "Too many neighbours added to particle " << i_source << '\n';
            err_msg << "This particle's adjacency list is full.\n";
            throw std::runtime_error(err_msg.str());
        }
    }
};

}  // namespace mathtools
