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

namespace impl_io
{

template <std::floating_point FP>
auto histogram_file_header_(const Histogram<FP>& histogram) -> std::string
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
void write_histogram_(std::ostream& out_stream, const Histogram<FP>& histogram)
{
    // everything except the bin values is already in the file header
    out_stream << histogram_file_header_(histogram);

    for (auto bin : histogram.bins()) {
        out_stream << bin << '\n';
    }
}

auto open_filestream_checked_(const std::filesystem::path& filepath, std::ios::openmode mode) -> std::ofstream
{
    auto out_stream = std::ofstream {filepath, mode};
    if (!out_stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to open file: " << filepath.string() << '\n';
        throw std::ios_base::failure {err_msg.str()};
    }

    return out_stream;
}

template <std::floating_point FP>
void write_new_histogram_(const std::filesystem::path& savepath, const Histogram<FP>& histogram)
{
    auto out_stream = open_filestream_checked_(savepath, std::ios::out);
    write_histogram_(out_stream, histogram);
}

}  // namespace impl_io

namespace io
{

template <std::floating_point FP>
void write_histogram(std::ostream& out_stream, const Histogram<FP>& histogram)
{
    impl_io::write_histogram_(out_stream, histogram);
}

template <std::floating_point FP>
void write_histogram(const std::filesystem::path& savepath, const Histogram<FP>& histogram)
{
    namespace fs = std::filesystem;

    auto temp_savepath = savepath;
    temp_savepath += common_utils::writer_utils::DEFAULT_TEMPORARY_SUFFIX;

    if (!fs::exists(savepath)) {
        impl_io::write_new_histogram_(savepath, histogram);
    }

    fs::copy_file(savepath, temp_savepath, fs::copy_options::overwrite_existing);
    impl_io::write_new_histogram_(temp_savepath, histogram);
    fs::rename(temp_savepath, savepath);
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

}  // namespace io

}  // namespace mathtools
