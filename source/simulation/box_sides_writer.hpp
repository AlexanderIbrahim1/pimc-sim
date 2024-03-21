#pragma once

#include <cstddef>
#include <concepts>
#include <filesystem>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <coordinates/box_sides.hpp>


namespace sim
{

constexpr inline auto BOX_SIDES_COORD_OUTPUT_PRECISION_ = int {8};

inline auto box_sides_header() -> std::string {
    auto message = std::stringstream {};

    message << "# this file contains information about the sides of the periodic box used in the simulation\n";
    message << "# the first uncommented line contains the number of dimensions\n";
    message << "# the following lines contain the side lengths, in order of the axis they belong to\n";
    message << "# for example, in 3D there would be 4 lines:\n";
    message << "# the first has the integer 3, and the next three are the x-axis, y-axis, and z-axis lengths, respectively\n";

    return message.str();
}

template <std::floating_point FP, std::size_t NDIM>
void write_box_sides(const std::filesystem::path& filepath, const coord::BoxSides<FP, NDIM>& box_sides) {
    auto out_stream = std::ofstream {filepath, std::ios::out};
    if (!out_stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Error: Unable to open file: '" << filepath << "'\n";
        throw std::ios_base::failure {err_msg.str()};
    }

    out_stream << box_sides_header();
    out_stream << NDIM << '\n';
    out_stream << std::scientific << std::setprecision(BOX_SIDES_COORD_OUTPUT_PRECISION_);
    for (auto coord : box_sides.coordinates()) {
        out_stream << coord << '\n';
    }
}

}  // namespace sim
