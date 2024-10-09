#pragma once

#include <concepts>
#include <cstddef>
#include <string_view>
#include <vector>

#include <common/common_utils.hpp>
#include <mathtools/mathtools_utils.hpp>

namespace mathtools
{

// NOTE: I'm not too concerned with the allocations from resizing
template <common::Numeric Number>
class Grid2D
{
public:
    Grid2D(std::size_t n_rows, std::size_t n_cols)
        : n_rows_ {n_rows}
        , n_cols_ {n_cols}
    {
        mathtools_utils::ctr_check_positive(n_rows, "xsize");
        mathtools_utils::ctr_check_positive(n_cols, "ysize");

        data_.resize(n_rows_ * n_cols_);
    }

    constexpr auto get(std::size_t i_row, std::size_t i_col) const noexcept -> Number
    {
        const auto index = i_col + i_row * n_cols_;
        return data_[index];
    }

    constexpr void set(std::size_t i_row, std::size_t i_col, Number value)
    {
        const auto index = i_col + i_row * n_cols_;
        data_[index] = value;
    }

    constexpr auto data() const noexcept -> const std::vector<Number>&
    {
        return data_;
    }

    constexpr auto n_rows() const noexcept -> std::size_t
    {
        return n_rows_;
    }

    constexpr auto n_cols() const noexcept -> std::size_t
    {
        return n_cols_;
    }

private:
    std::size_t n_rows_;
    std::size_t n_cols_;
    std::vector<Number> data_;
};

}  // namespace mathtools
