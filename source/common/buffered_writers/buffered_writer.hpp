#pragma once

#include <array>
#include <concepts>
#include <fstream>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <common/common_utils.hpp>
#include <common/io_utils.hpp>
#include <common/writer_utils.hpp>

#include <common/buffered_writers/format_info.hpp>

namespace impl_block_value_writer
{

namespace cw = common::writers;

template <std::size_t N>
auto default_format_info() noexcept -> cw::FormatInfo<N>
{
    static_assert(N >= 1, "FormatInfo<N> must be constructed with at least one argument.");

    namespace cw = common::writers;

    const auto block_index_padding = cw::DEFAULT_WRITER_BLOCK_INDEX_PADDING;
    const auto spacing = cw::DEFAULT_SPACING;

    auto integer_padding = std::array<int, N> {};
    integer_padding.fill(cw::DEFAULT_WRITER_INTEGER_PADDING);

    auto floating_point_precision = std::array<int, N> {};
    floating_point_precision.fill(cw::DEFAULT_WRITER_FLOATING_POINT_PRECISION);

    return cw::FormatInfo<N> {block_index_padding, spacing, integer_padding, floating_point_precision};
}

template <std::size_t Index, std::size_t N, typename Tuple>
void format_value(std::ostream& stream, const Tuple& tuple, const cw::FormatInfo<N> format_info) {
    static_assert(N + 1 == std::tuple_size<Tuple>::value, "FormatInfo must be the correct size to be able to format lines.");

    // avoid iterating beyond the end of the tuple
    if constexpr (Index < std::tuple_size<Tuple>::value) {
        // the value that we want to write
        const auto value = std::get<Index>(tuple);

        // the padding comes first
        stream << std::string (format_info.spacing, ' ');

        // apply different types of formatting, depending on whether the output is a floating-point type or an integer
        using ElementType = typename std::tuple_element<Index, Tuple>::type;
        if constexpr (std::is_floating_point<ElementType>::value) {
            const auto precision = format_info.floating_point_precision[Index - 1];
            stream << std::scientific << std::setprecision(precision) << value;
        }
        else {
            const auto padding = format_info.integer_padding[Index - 1];
            stream << std::setw(padding) << std::setfill(' ') << std::right << value;
        }

        format_value<Index + 1, N, Tuple>(stream, tuple, format_info);
    }
}

template <common::Numeric... Number>
class BufferedStreamValueWriter
{
public:
    using Data = std::tuple<std::size_t, Number...>;
    static constexpr auto NumValues = std::tuple_size<Data>::value - std::size_t {1};

    static_assert(NumValues >= 1, "Data must contain at least two elements.");

    void accumulate(const Data& data)
    {
        buffered_data_.emplace_back(data);
    }

    void write_and_clear(std::ostream& out_stream, const cw::FormatInfo<NumValues>& format_info)
    {
        auto lines_stream = std::stringstream {};

        for (const auto& data : buffered_data_) {
            lines_stream << formatted_line_(data, format_info);
        }

        out_stream << lines_stream.str();

        buffered_data_.clear();
    }

    auto is_buffer_empty() const noexcept -> bool
    {
        return buffered_data_.empty();
    }

private:
    std::vector<Data> buffered_data_ {};

    auto formatted_line_(const Data& data, const cw::FormatInfo<NumValues>& format_info) const -> std::string
    {
        auto line_stream = std::stringstream {};
        line_stream << std::setw(format_info.block_index_padding) << std::setfill('0') << std::right << std::get<0>(data);

        format_value<1, NumValues, Data>(line_stream, data, format_info);

        line_stream << '\n';

        return line_stream.str();
    }
};

}  // namespace impl_block_value_writer


namespace common::writers
{

template <common::Numeric... Number>
class BlockValueWriter
{
public:
    using Data = std::tuple<std::size_t, Number...>;
    static constexpr auto NumValues = std::tuple_size<Data>::value - std::size_t {1};

    static_assert(NumValues >= 1, "Data must contain at least two elements.");

    BlockValueWriter(
        std::filesystem::path filepath,
        std::string header_contents = std::string {},
        FormatInfo<NumValues> format_info = impl_block_value_writer::default_format_info<NumValues>()
    )
        : filepath_ {std::move(filepath)}
        , header_contents_ {std::move(header_contents)}
        , format_info_ {std::move(format_info)}
        , stream_writer_ {}
    {}

    void accumulate(const Data& data)
    {
        stream_writer_.accumulate(data);
    }

    void write_and_clear()
    {
        if (stream_writer_.is_buffer_empty()) {
            return;
        }

        // performs an atomic write of the new data
        namespace fs = std::filesystem;

        auto temp_filepath = filepath_;
        temp_filepath += common::writers::DEFAULT_TEMPORARY_SUFFIX;

        if (!fs::exists(filepath_)) {
            write_first_();
        }

        fs::copy_file(filepath_, temp_filepath, fs::copy_options::overwrite_existing);
        write_and_clear_(temp_filepath);
        fs::rename(temp_filepath, filepath_);
    }

    void write_nonatomic()
    {
        namespace fs = std::filesystem;

        if (!fs::exists(filepath_)) {
            write_first_();
        }

        write_and_clear_(filepath_);
    }

private:
    std::filesystem::path filepath_;
    std::string header_contents_;
    FormatInfo<NumValues> format_info_;
    impl_block_value_writer::BufferedStreamValueWriter<Number...> stream_writer_;

    void write_and_clear_(const std::filesystem::path& filepath)
    {
        auto out_stream = common::io::open_append_filestream_checked(filepath);
        stream_writer_.write_and_clear(out_stream, format_info_);
    }

    void write_first_() const
    {
        auto out_stream = common::io::open_output_filestream_checked(filepath_);
        out_stream << header_contents_;
    }
};

}  // namespace common::writers
