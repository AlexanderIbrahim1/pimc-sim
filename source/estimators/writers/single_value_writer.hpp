#pragma once

#include <concepts>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <sstream>

#include <common/common_utils.hpp>
#include <estimators/estim_utils.hpp>

namespace estim
{

template <common_utils::Numeric Number>
class SingleValueBlockWriter
{
public:
    constexpr SingleValueBlockWriter(std::filesystem::path filepath, std::string header_contents = std::string {})
        : filepath_ {std::move(filepath)}
        , block_index_padding_ {estim_utils::DEFAULT_BLOCK_INDEX_PADDING}
        , floating_point_precision_ {estim_utils::DEFAULT_SINGLE_VALUE_PRECISION}
    {
        auto out_stream = open_filestream_checked_(std::ios::out);
        out_stream << header_contents;
    }

    void write(std::size_t i_block, Number value) const
    {
        auto out_stream = open_filestream_checked_(std::ios::app);

        out_stream << std::setw(block_index_padding_) << std::setfill('0') << std::right << i_block << "   ";

        // apply different types of formatting, depending on whether the output is a floating-point type or an integer
        if constexpr (std::is_floating_point_v<Number>) {
            out_stream << std::scientific << std::setprecision(floating_point_precision_) << value << '\n';
        }
        else {
            out_stream << value << '\n';
        }
    }

    constexpr void set_block_index_padding(int block_index_padding) const noexcept
    {
        block_index_padding_ = block_index_padding;
    }

    constexpr void set_floating_point_precision(int floating_point_precision) const noexcept
    {
        // want to prevent user from possibly passing in a negative precision
        floating_point_precision_ = floating_point_precision;
    }

private:
    std::filesystem::path filepath_;
    int block_index_padding_;
    int floating_point_precision_;

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

}  // namespace estim
