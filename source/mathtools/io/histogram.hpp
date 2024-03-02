#pragma once

#include <concepts>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <common/writers/writer_utils.hpp>
#include <mathtools/histogram/histogram.hpp>

namespace mathtools
{

namespace io
{

static constexpr auto policy_to_int(OutOfRangePolicy policy) -> int
{
    // currently only two policies
    if (policy == OutOfRangePolicy::DO_NOTHING) {
        return 0;
    }
    else {
        return 1;
    }
}

template <std::floating_point FP>
auto histogram_file_header(const Histogram<FP>& histogram) -> std::string
{
    auto header = std::stringstream {};
    header << "# This file contains the state of a regularly-spaced histogram";
    header << "# The layout for the histogram data is as follows:\n";
    header << "# - [integer] the out-of-range policy (0 = DO_NOTHING, 1 = THROW)\n";
    header << "# - [integer] the number of bins\n";
    header << "# - [floating-point] the minimum value\n";
    header << "# - [floating-point] the maximum value\n";
    header << "# ... followed by the count in each histogram bin, in single-column order...\n ";

    header << policy_to_int(histogram.policy()) << '\n';
    header << histogram.bins().size() << '\n';

    auto precision = common_utils::writer_utils::DEFAULT_WRITER_SINGLE_VALUE_PRECISION;

    header << std::scientific << std::setprecision(precision);
    header << histogram.min() << '\n';
    header << histogram.max() << '\n';

    return header.str();
}

template <std::floating_point FP>
void write_histogram(const std::filesystem::path& savepath, const Histogram<FP>& histogram)
{
    auto out_stream = std::ofstream {savepath, std::ios::out};
    if (!out_stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Error: Unable to open file: '" << savepath << "'\n";
        throw std::ios_base::failure {err_msg.str()};
    }

    write_histogram(out_stream, histogram);
}

template <std::floating_point FP>
void write_histogram(std::ostream& out_stream, const Histogram<FP>& histogram)
{
    out_stream << histogram_file_header(histogram);

    for (auto bin : histogram.bins()) {
        out_stream << bin << '\n';
    }
}

}  // namespace io

}  // namespace mathtools
