#pragma once

#include <filesystem>
#include <fstream>
#include <ios>
#include <sstream>
#include <stdexcept>

namespace impl_common
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

}  // namespace impl_common

namespace common
{

namespace io
{

inline auto open_output_filestream_checked(const std::filesystem::path& filepath) -> std::ofstream
{
    return impl_common::open_output_filestream_checked_(filepath, std::ios::out);
}

inline auto open_append_filestream_checked(const std::filesystem::path& filepath) -> std::ofstream
{
    return impl_common::open_output_filestream_checked_(filepath, std::ios::app);
}

inline auto open_input_filestream_checked(const std::filesystem::path& filepath) -> std::ifstream
{
    auto in_stream = std::ifstream {filepath};
    if (!in_stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to open file for reading from: " << filepath.string() << '\n';
        throw std::ios_base::failure {err_msg.str()};
    }

    return in_stream;
}

}  // namespace io

}  // namespace common
