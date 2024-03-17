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

template <std::floating_point FP>
auto histogram_file_header(const Histogram<FP>& histogram) -> std::string
{
    auto header = std::stringstream {};
    header << "# This file contains the state of a regularly-spaced histogram\n";
    header << "# The layout for the histogram data is as follows:\n";
    header << "# - [integer] the out-of-range policy (0 = DO_NOTHING, 1 = THROW)\n";
    header << "# - [integer] the number of bins\n";
    header << "# - [floating-point] the minimum value\n";
    header << "# - [floating-point] the maximum value\n";
    header << "# ... followed by the count in each histogram bin, in single-column order...\n";

    header << static_cast<int>(histogram.policy()) << '\n';
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
    // everything except the bin values is already in the file header
    out_stream << histogram_file_header(histogram);

    for (auto bin : histogram.bins()) {
        out_stream << bin << '\n';
    }
}

template <std::floating_point FP>
auto read_histogram(const std::filesystem::path& loadpath) -> Histogram<FP>
{
    auto in_stream = std::ifstream {loadpath, std::ios::in};
    if (!in_stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Error: Unable to open file: '" << loadpath << "'\n";
        throw std::ios_base::failure {err_msg.str()};
    }

    return read_histogram<FP>(in_stream);
}

template <std::floating_point FP>
auto read_histogram(std::istream& in_stream) -> Histogram<FP>
{
    common_utils::writer_utils::skip_lines_starting_with(in_stream, '#');

    auto policy_key = int {};
    auto n_bins = std::size_t {};
    auto min = FP {};
    auto max = FP {};
    auto count = std::uint64_t {};

    in_stream >> policy_key;
    in_stream >> n_bins;
    in_stream >> min;
    in_stream >> max;

    std::vector<std::uint64_t> bins;
    bins.reserve(n_bins);
    for (std::size_t i {0}; i < n_bins; ++i) {
        in_stream >> count;
        bins.push_back(count);
    }

    return Histogram<FP> {min, max, std::move(bins), static_cast<OutOfRangePolicy>(policy_key)};
}

}  // namespace io

}  // namespace mathtools
