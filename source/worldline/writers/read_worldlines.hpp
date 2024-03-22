#pragma once

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <common/writers/writer_utils.hpp>
#include <coordinates/cartesian.hpp>
#include <worldline/worldline.hpp>
#include <worldline/writers/worldline_writer.hpp>

namespace worldline
{

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
