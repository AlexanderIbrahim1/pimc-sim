#pragma once

#include <concepts>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <sstream>

#include <common/common_utils.hpp>
#include <common/writers/writer_utils.hpp>

namespace common
{

namespace writers
{

template <common_utils::Numeric Number>
class SingleValueBlockWriter
{
public:
    constexpr SingleValueBlockWriter(std::filesystem::path filepath, std::string header_contents = std::string {})
        : filepath_ {std::move(filepath)}
    {
        auto out_stream = open_filestream_checked_(std::ios::out);
        out_stream << header_contents;
    }

    void write(std::size_t i_block, Number value) const
    {
        auto out_stream = open_filestream_checked_(std::ios::app);

        out_stream << std::setw(block_index_padding) << std::setfill('0') << std::right << i_block << spacing_;

        // apply different types of formatting, depending on whether the output is a floating-point type or an integer
        if constexpr (std::is_floating_point_v<Number>) {
            out_stream << std::scientific << std::setprecision(floating_point_precision) << value << '\n';
        }
        else {
            out_stream << std::setw(integer_padding) << std::setfill(' ') << std::right << value << '\n';
        }
    }

    int block_index_padding {common_utils::writer_utils::DEFAULT_WRITER_BLOCK_INDEX_PADDING};
    int floating_point_precision {common_utils::writer_utils::DEFAULT_WRITER_SINGLE_VALUE_PRECISION};
    int integer_padding {common_utils::writer_utils::DEFAULT_WRITER_INTEGER_PADDING};

private:
    std::filesystem::path filepath_;
    std::string spacing_ {"   "};

    auto open_filestream_checked_(std::ios::openmode mode) const -> std::ofstream
    {
        auto out_stream = std::ofstream {filepath_, mode};
        if (!out_stream.is_open()) {
            auto err_msg = std::stringstream {};
            err_msg << "Failed to open file: " << filepath_.string() << '\n';
            throw std::ios_base::failure {err_msg.str()};
        }

        return out_stream;
    }
};

}  // namespace writers

}  // namespace common
