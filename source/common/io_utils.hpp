#pragma once

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <stdexcept>

namespace impl_common_utils_io
{

inline auto open_output_filestream_checked_(const std::filesystem::path& filepath, std::ios::openmode mode)
    -> std::ofstream
{
    auto out_stream = std::ofstream {filepath, mode};
    if (!out_stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to open file: " << filepath.string() << '\n';
        throw std::ios_base::failure {err_msg.str()};
    }

    return out_stream;
}

}  // namespace impl_common_utils_io

namespace common_utils
{

inline auto open_output_filestream_checked(const std::filesystem::path& filepath) -> std::ofstream
{
    return impl_common_utils_io::open_output_filestream_checked_(filepath, std::ios::out);
}

auto open_append_filestream_checked(const std::filesystem::path& filepath) -> std::ofstream
{
    return impl_common_utils_io::open_output_filestream_checked_(filepath, std::ios::app);
}

auto open_input_filestream_checked(const std::filesystem::path& filepath) -> std::ifstream
{
    auto in_stream = std::ifstream {filepath};
    if (!in_stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to open file for reading from: " << filepath.string() << '\n';
        throw std::ios_base::failure {err_msg.str()};
    }

    return in_stream;
}

}  // namespace common_utils
