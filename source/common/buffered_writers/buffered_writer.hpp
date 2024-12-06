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

namespace common
{

namespace writers
{

template <std::size_t N>
struct FormatInfo
{
    int block_index_padding;
    int spacing;
    std::array<int, N> integer_padding;
    std::array<int, N> floating_point_precision;
};

template <std::size_t Index, std::size_t N, typename Tuple>
void format_value(std::ostream& stream, const Tuple& tuple, const FormatInfo<N> format_info) {
    static_assert(N + 1 == std::tuple_size<Data>::value, "FormatInfo must be the correct size to be able to format lines.");

    // avoid iterating beyond the end of the tuple
    if constexpr (Index >= std::tuple_size<Tuple>::value) {
        return;
    }

    // the value that we want to write
    const auto value = std::get<Index>(tuple);

    // apply different types of formatting, depending on whether the output is a floating-point type or an integer
    using ElementType = typename std::tuple_element<Index, Tuple>::type;
    if constexpr (std::is_floating_point<ElementType>::value) {
        const auto precision = format_info.floating_point_precision[Index - 1];
        line_stream << std::scientific << std::setprecision(precision) << value << '\n';
    }
    else {
        const auto padding = format_info.integer_padding[Index - 1];
        line_stream << std::setw(padding) << std::setfill(' ') << std::right << value << '\n';
    }

    format_value<Index + 1, N, Tuple>(stream, tuple, format_info);
}

template <common::Numeric... Number>
class BufferedStreamValueWriter
{
public:
    using Data = std::tuple<std::size_t, Number...>;

    static_assert(std::tuple_size<Data>::value >= 2, "Data must contain at least two elements.");

    void accumulate(const Data& data)
    {
        buffered_data_.emplace_back(data);
    }

    template <std::size_t N>
    void write_and_clear(std::ostream& out_stream, const FormatInfo<N>& format_info)
    {
        static_assert(N + 1 == std::tuple_size<Data>::value, "FormatInfo must be the correct size to be able to format lines.");

        auto lines_stream = std::stringstream {};

        for (const auto& data : buffered_data_) {
            lines_stream << formatted_line_(data);
        }

        out_stream << lines_stream.str();

        buffered_data_.clear();
    }

private:
    std::string spacing_ {"   "};
    std::vector<Data> buffered_data_ {};

    template <std::size_t N>
    auto formatted_line_(const Data& data, const FormatInfo<N>& format_info) const -> std::string
    {
        auto line_stream = std::stringstream {};
        line_stream << std::setw(format_info.block_index_padding) << std::setfill('0') << std::right;
        line_stream << std::get<0>(data) << format_info.spacing;

        format_value<1, N, Data>(line_stream, data, format_info);
    }
};

}  // namespace writers

}  // namespace common
