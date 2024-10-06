#pragma once

// TODO: remove
#include <iostream>

#include <concepts>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <optional>
#include <vector>

#include <common/writers/writer_utils.hpp>
#include <constants/constants.hpp>
#include <interactions/three_body/three_body_parah2.hpp>
#include <mathtools/mathtools_utils.hpp>
#include <mathtools/grid/grid3d.hpp>
#include <mathtools/interpolate/trilinear_interp.hpp>

namespace interact
{

template <std::floating_point FP>
auto three_body_ibrahim2022(const std::filesystem::path& data_filepath, const std::optional<FP>& c9_coefficient = std::nullopt)
{
    auto instream = std::ifstream(data_filepath, std::ios::in);
    if (!instream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Error: Unable to open file for trilinear interpolation data: '" << data_filepath << "'\n";
        throw std::ios_base::failure {err_msg.str()};
    }

    common_utils::writer_utils::skip_lines_starting_with(instream, '#');

    // arguments (r, s, u) correspond to coordinates (R, s, cos(phi)) in the published paper
    std::size_t r_size, s_size, u_size;
    instream >> r_size;
    instream >> s_size;
    instream >> u_size;

    FP r_min, r_max, s_min, s_max, u_min, u_max;
    instream >> r_min;
    instream >> r_max;
    instream >> s_min;
    instream >> s_max;
    instream >> u_min;
    instream >> u_max;

    std::cout << "(r_size, s_size, u_size) = (" << r_size << ", " << s_size << ", " << u_size << ")\n";
    std::cout << "r_min = " << r_min << '\n';
    std::cout << "r_max = " << r_max << '\n';
    std::cout << "s_min = " << s_min << '\n';
    std::cout << "s_max = " << s_max << '\n';
    std::cout << "u_min = " << u_min << '\n';
    std::cout << "u_max = " << u_max << '\n';

    const auto shape = mathtools::Shape3D {r_size, s_size, u_size};
    const auto r_limits = mathtools_utils::AxisLimits {r_min, r_max};
    const auto s_limits = mathtools_utils::AxisLimits {s_min, s_max};
    const auto u_limits = mathtools_utils::AxisLimits {u_min, u_max};

    // read in all the energies into a vector
    const auto n_elements = r_size * s_size * u_size;
    auto energies = std::vector<FP> {};
    energies.reserve(n_elements);

    FP energy;
    for (std::size_t i {0}; i < n_elements; ++i) {
        instream >> energy;
        energies.push_back(energy);
    }

    std::cout << "energies[0] = " << energies[0] << '\n';
    std::cout << "energies[n_elements - 1] = " << energies[n_elements - 1] << '\n';

    // create the 3D grid of energies to perform trilinear interpolation on
    auto grid = mathtools::Grid3D {std::move(energies), shape};

    auto interpolator = mathtools::TrilinearInterpolator<FP> {std::move(grid), r_limits, s_limits, u_limits};
    const auto coefficient = c9_coefficient.value_or(constants::C9_ATM_COEFFICIENT_HINDE2008<FP>);

    return interact::ThreeBodyParaH2Potential {std::move(interpolator), coefficient};
}

}  // namespace interact
