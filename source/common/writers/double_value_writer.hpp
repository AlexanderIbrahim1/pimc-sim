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

template <common_utils::Numeric Number1, common_utils::Numeric Number2>
class DoubleValueBlockWriter
{
public:
    constexpr DoubleValueBlockWriter(std::filesystem::path filepath, std::string header_contents = std::string {})
        : filepath_ {std::move(filepath)}
        , header_contents_ {std::move(header_contents)}
    {}

    void write(std::size_t i_block, Number1 value1, Number2 value2) const
    {
        // performs an atomic write of the new data
        namespace fs = std::filesystem;

        auto temp_filepath = filepath_;
        temp_filepath += common_utils::writer_utils::DEFAULT_TEMPORARY_SUFFIX;

        if (!fs::exists(filepath_)) {
            write_first_();
        }

        fs::copy_file(filepath_, temp_filepath, fs::copy_options::overwrite_existing);
        write_(temp_filepath, i_block, value1, value2);
        fs::rename(temp_filepath, filepath_);
    }

    void write_nonatomic(std::size_t i_block, Number1 value1, Number2 value2) const
    {
        namespace fs = std::filesystem;

        if (!fs::exists(filepath_)) {
            write_first_();
        }

        write_(filepath_, i_block, value1, value2);
    }

    int block_index_padding {common_utils::writer_utils::DEFAULT_WRITER_BLOCK_INDEX_PADDING};
    int value1_integer_padding {common_utils::writer_utils::DEFAULT_WRITER_INTEGER_PADDING};
    int value2_integer_padding {common_utils::writer_utils::DEFAULT_WRITER_INTEGER_PADDING};
    int value1_floating_point_precision {common_utils::writer_utils::DEFAULT_WRITER_SINGLE_VALUE_PRECISION};
    int value2_floating_point_precision {common_utils::writer_utils::DEFAULT_WRITER_SINGLE_VALUE_PRECISION};

private:
    std::filesystem::path filepath_;
    std::string header_contents_;
    std::string spacing_ {"   "};

    auto open_filestream_checked_(const std::filesystem::path& filepath, std::ios::openmode mode) const -> std::ofstream
    {
        auto out_stream = std::ofstream {filepath, mode};
        if (!out_stream.is_open()) {
            auto err_msg = std::stringstream {};
            err_msg << "Failed to open file: " << filepath.string() << '\n';
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

    void write_(const std::filesystem::path& filepath, std::size_t i_block, Number1 value1, Number2 value2) const
    {
        auto out_stream = open_filestream_checked_(filepath, std::ios::app);

        out_stream << std::setw(block_index_padding) << std::setfill('0') << std::right << i_block;
        out_stream << spacing_;
        output_value_<Number1>(out_stream, value1, value1_floating_point_precision, value1_integer_padding);
        out_stream << spacing_;
        output_value_<Number2>(out_stream, value2, value2_floating_point_precision, value2_integer_padding);
        out_stream << '\n';
    }

    void write_first_() const
    {
        auto out_stream = open_filestream_checked_(filepath_, std::ios::out);
        out_stream << header_contents_;
    }
};

}  // namespace writers

}  // namespace common
