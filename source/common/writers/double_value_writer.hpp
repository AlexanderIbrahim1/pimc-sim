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

template <common_utils::Numeric Number1, common_utils::Numeric Number2>
class DoubleValueBlockWriter
{
public:
    constexpr DoubleValueBlockWriter(std::filesystem::path filepath, std::string header_contents = std::string {})
        : filepath_ {std::move(filepath)}
    {
        auto out_stream = open_filestream_checked_(std::ios::out);
        out_stream << header_contents;
    }

    void write(std::size_t i_block, Number1 value1, Number2 value2) const
    {
        auto out_stream = open_filestream_checked_(std::ios::app);

        out_stream << std::setw(block_index_padding) << std::setfill('0') << std::right << i_block;
        out_stream << spacing_;
        output_value_<Number1>(out_stream, value1, value1_floating_point_precision, value1_integer_padding);
        out_stream << spacing_;
        output_value_<Number2>(out_stream, value2, value2_floating_point_precision, value2_integer_padding);
        out_stream << '\n';
    }

    int block_index_padding {common_utils::writer_utils::DEFAULT_WRITER_BLOCK_INDEX_PADDING};
    int value1_integer_padding {common_utils::writer_utils::DEFAULT_WRITER_INTEGER_PADDING};
    int value2_integer_padding {common_utils::writer_utils::DEFAULT_WRITER_INTEGER_PADDING};
    int value1_floating_point_precision {common_utils::writer_utils::DEFAULT_WRITER_SINGLE_VALUE_PRECISION};
    int value2_floating_point_precision {common_utils::writer_utils::DEFAULT_WRITER_SINGLE_VALUE_PRECISION};

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

    template <common_utils::Numeric Number>
    auto output_value_(std::ofstream& out_stream, Number value, int fp_precision, int int_padding) const
    {
        if constexpr (std::is_floating_point_v<Number>) {
            out_stream << std::scientific << std::setprecision(fp_precision) << value;
        }
        else {
            out_stream << std::setw(int_padding) << std::setfill(' ') << std::right << value;
        }
    }
};

}  // namespace common
