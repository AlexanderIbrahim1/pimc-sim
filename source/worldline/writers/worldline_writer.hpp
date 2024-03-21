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

#include <common/writers/writer_utils.hpp>
#include <coordinates/box_sides.hpp>
#include <environment/environment.hpp>
#include <worldline/worldline.hpp>

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
static auto worldline_file_header(std::size_t n_particles, std::size_t n_timeslices, std::size_t i_block) noexcept
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

template <std::floating_point FP, std::size_t NDIM>
class _WorldlineWriterImpl
{
public:
    explicit _WorldlineWriterImpl(
        std::filesystem::path output_dirpath,
        std::string prefix = std::string {"worldline"},
        std::string suffix = std::string {".dat"}
    )
        : output_dirpath_ {std::move(output_dirpath)}
        , prefix_ {std::move(prefix)}
        , suffix_ {std::move(suffix)}
    {}

    void write(std::size_t i_block, std::string header, const std::vector<worldline::Worldline<FP, NDIM>>& worldlines)
        const
    {
        const auto output_filepath_ = output_filepath(i_block);

        auto out_stream = std::ofstream {output_filepath_, std::ios::out};
        if (!out_stream.is_open()) {
            auto err_msg = std::stringstream {};
            err_msg << "Failed to open file: " << output_filepath_.string() << '\n';
            throw std::ios_base::failure {err_msg.str()};
        }

        out_stream << header;

        const auto precision = common_utils::writer_utils::DEFAULT_WRITER_SINGLE_VALUE_PRECISION;
        out_stream << std::scientific << std::setprecision(precision);

        for (const auto& worldline : worldlines) {
            for (auto point : worldline.points()) {
                for (std::size_t i_dim {0}; i_dim < NDIM; ++i_dim) {
                    const auto value = point[i_dim];

                    // replicates the "space or negative sign" formatting from Python
                    if (value >= FP {0.0}) {
                        out_stream << ' ';
                    }

                    out_stream << value;

                    if (i_dim != NDIM - 1) {
                        out_stream << writer_utils::POSITION_SPACING;
                    }
                }
                out_stream << '\n';
            }
        }
    }

    auto output_filepath(std::size_t i_block) const -> std::filesystem::path
    {
        const auto padding = common_utils::writer_utils::DEFAULT_WRITER_BLOCK_INDEX_PADDING;

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

template <std::floating_point FP, std::size_t NDIM>
class WorldlineWriter
{
public:
    explicit WorldlineWriter(
        std::filesystem::path output_dirpath,
        std::string prefix = std::string {"worldline"},
        std::string suffix = std::string {".dat"}
    )
        : worldline_writer_ {std::move(output_dirpath), std::move(prefix), std::move(suffix)}
    {}

    void write(
        std::size_t i_block,
        const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
        const envir::Environment<FP> environment
    ) const
    {
        const auto n_particles = environment.n_particles();
        const auto n_timeslices = environment.n_timeslices();
        auto header = worldline_file_header<FP, NDIM>(n_particles, n_timeslices, i_block);

        worldline_writer_.write(i_block, std::move(header), worldlines);
    }

    auto output_filepath(std::size_t i_block) const -> std::filesystem::path
    {
        return worldline_writer_.output_filepath(i_block);
    }

private:
    _WorldlineWriterImpl<FP, NDIM> worldline_writer_;
};

//    header << "# This file contains the positions of all the beads in all the particles in a simulation\n";
//    header << "# The information after the comments is laid out in the following manner:\n";
//    header << "# - [integer] block index of the simulation this snapshot is taken at\n";
//    header << "# - [integer] NDIM: number of dimensions the simulation was performed in\n";
//    header << "# - [integer] n_particles: total number of particles\n";
//    header << "# - [integer] n_timeslices: total number of timeslices\n";
//    header << "# ... followed by the bead positions...\n";
//    header << "# \n";
//    header << "# The positions of the beads are laid out in `NDIM` space-separated columns;\n";
//    header << "#   - the first `n_particle` lines correspond to the 0th worldline\n";
//    header << "#   - the next `n_particle` lines correspond to the 1st worldline\n";
//    header << "#   - the next `n_particle` lines correspond to the 2nd worldline, and so on\n";
//    header << "#   - there are `n_timeslices` worldlines in total\n";
//    header << i_block << '\n';
//    header << NDIM << '\n';
//    header << n_particles << '\n';
//    header << n_timeslices << '\n';

template <std::floating_point FP, std::size_t NDIM>
auto read_cartesian(std::istream& stream) -> coord::Cartesian<FP, NDIM>
{
    std::array<FP, NDIM> coordinates;
    for (std::size_t i {0}; i < NDIM; ++i) {
        stream >> coordinates[i];
    }

    return coord::Cartesian<FP, NDIM> {std::move(coordinates)};
}

template <std::floating_point FP, std::size_t NDIM>
auto read_worldlines(std::istream& stream) -> std::vector<worldline::Worldline<FP, NDIM>>
{
    common_utils::writer_utils::skip_lines_starting_with(stream, '#');

    // the first non-comment line is the block index; we don't really need it
    // not enough lines for std::istream::ignore to be worth it
    auto dummy = std::string {};
    std::getline(stream, dummy);

    // get the number of dimensions; might as well do a verification
    auto ndim = std::size_t {};
    stream >> ndim;

    if (NDIM != ndim) {
        auto err_msg = std::stringstream {};
        err_msg
            << "The number of dimensions for this simulation does not match the number of dimensions in the file.\n";
        err_msg << "In simulation: NDIM = " << NDIM << '\n';
        err_msg << "In file: ndim = " << ndim << '\n';
        throw std::runtime_error {err_msg.str()};
    }

    // next are the number of particles and timeslices
    auto n_particles = std::size_t {};
    auto n_timeslices = std::size_t {};

    stream >> n_particles;
    stream >> n_timeslices;

    auto worldlines = std::vector<worldline::Worldline<FP, NDIM>> {};
    worldlines.reserve(n_timeslices);
    for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
        auto points = std::vector<coord::Cartesian<FP, NDIM>> {};
        points.reserve(n_particles);
        for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
            points.emplace_back(read_cartesian<FP, NDIM>(stream));
        }
        worldlines.emplace_back(std::move(points));
    }

    return worldlines;
}

template <std::floating_point FP, std::size_t NDIM>
auto read_worldlines(const std::filesystem::path& filepath) -> std::vector<worldline::Worldline<FP, NDIM>>
{
    auto stream = std::ifstream {filepath};
    if (!stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Error: Unable to open file: '" << filepath << "'\n";
        throw std::ios_base::failure {err_msg.str()};
    }

    return read_worldlines<FP, NDIM>(stream);
}

}  // namespace worldline
