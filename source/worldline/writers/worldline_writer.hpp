#pragma once

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <common/io_utils.hpp>
#include <common/writers/writer_utils.hpp>
#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <environment/environment.hpp>
#include <worldline/worldline.hpp>

namespace impl_worldline
{

template <std::floating_point FP, std::size_t NDIM>
auto formatted_cartesian_line_for_worldline_file_(const coord::Cartesian<FP, NDIM>& point) -> std::string
{
    const auto precision = common::writers::DEFAULT_WRITER_SINGLE_VALUE_PRECISION;

    auto formatted_cartesian = std::stringstream {};
    formatted_cartesian << std::scientific << std::setprecision(precision);

    for (std::size_t i_dim {0}; i_dim < NDIM; ++i_dim) {
        const auto value = point[i_dim];

        // replicates the "space or negative sign" formatting from Python
        if (value >= FP {0.0}) {
            formatted_cartesian << ' ';
        }

        formatted_cartesian << value;

        if (i_dim != NDIM - 1) {
            formatted_cartesian << common::writers::DEFAULT_MULTICOLUMN_SPACES;
        }
    }
    formatted_cartesian << '\n';

    return formatted_cartesian.str();
}

template <std::floating_point FP, std::size_t NDIM>
static auto worldline_file_header_(std::size_t n_particles, std::size_t n_timeslices, std::size_t i_block) noexcept
    -> std::string
{
    auto header = std::stringstream {};
    header << "# This file contains the positions of all the beads in all the particles in a simulation\n";
    header << "# The information after the comments is laid out in the following manner:\n";
    header << "# - [integer] block index of the simulation this snapshot is taken at\n";
    header << "# - [integer] NDIM: number of dimensions the simulation was performed in\n";
    header << "# - [integer] n_particles: total number of particles\n";
    header << "# - [integer] n_timeslices: total number of timeslices\n";
    header << "# ... followed by the bead positions...\n";
    header << "# \n";
    header << "# The positions of the beads are laid out in `NDIM` space-separated columns;\n";
    header << "#   - the first `n_particle` lines correspond to the 0th worldline\n";
    header << "#   - the next `n_particle` lines correspond to the 1st worldline\n";
    header << "#   - the next `n_particle` lines correspond to the 2nd worldline, and so on\n";
    header << "#   - there are `n_timeslices` worldlines in total\n";
    header << i_block << '\n';
    header << NDIM << '\n';
    header << n_particles << '\n';
    header << n_timeslices << '\n';

    return header.str();
}

}  // namespace impl_worldline

namespace worldline
{

template <std::floating_point FP, std::size_t NDIM>
class WorldlineWriter
{
public:
    explicit WorldlineWriter(
        std::filesystem::path output_dirpath,
        std::string prefix = std::string {"worldline"},
        std::string suffix = std::string {".dat"}
    )
        : output_dirpath_ {std::move(output_dirpath)}
        , prefix_ {std::move(prefix)}
        , suffix_ {std::move(suffix)}
    {}

    void write(std::size_t i_block, const Worldlines<FP, NDIM>& worldlines) const
    {
        const auto n_particles = worldlines.n_worldlines();
        const auto n_timeslices = worldlines.n_timeslices();
        const auto header = impl_worldline::worldline_file_header_(n_particles, n_timeslices, i_block);

        const auto output_filepath_ = output_filepath(i_block);
        auto out_stream = common::io::open_output_filestream_checked(output_filepath_);

        out_stream << header;

        for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
            for (const auto& point : worldlines.timeslice(i_tslice)) {
                out_stream << impl_worldline::formatted_cartesian_line_for_worldline_file_(point);
            }
        }
    }

    auto output_filepath(std::size_t i_block) const -> std::filesystem::path
    {
        const auto padding = common::writers::DEFAULT_WRITER_BLOCK_INDEX_PADDING;

        auto filename = std::stringstream {};
        filename << prefix_;
        filename << std::setw(padding) << std::setfill('0') << std::right << i_block;
        filename << suffix_;

        return output_dirpath_ / filename.str();
    }

private:
    std::filesystem::path output_dirpath_;
    std::string prefix_;
    std::string suffix_;
};

}  // namespace worldline
