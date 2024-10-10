#pragma once

#include <concepts>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <vector>

#include <mathtools/grid/grid_iterator.hpp>
#include <mathtools/mathtools_utils.hpp>

namespace mathtools
{

// NOTE: I'm not too concerned with the allocations from resizing
template <typename T>
class Grid2D
{
public:
    static_assert(std::is_default_constructible<T>::value, "Template parameter 'T' must be default constructible.");

    Grid2D(std::size_t n_rows, std::size_t n_cols)
        : n_rows_ {n_rows}
        , n_cols_ {n_cols}
    {
        mathtools_utils::ctr_check_positive(n_rows, "xsize");
        mathtools_utils::ctr_check_positive(n_cols, "ysize");

        data_.resize(n_rows_ * n_cols_);
    }

    constexpr auto get(std::size_t i_row, std::size_t i_col) const noexcept -> T
    {
        const auto index = i_col + i_row * n_cols_;
        return data_[index];
    }

    constexpr void set(std::size_t i_row, std::size_t i_col, T value)
    {
        const auto index = i_col + i_row * n_cols_;
        data_[index] = value;
    }

    constexpr auto data() const noexcept -> const std::vector<T>&
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

    constexpr auto iterator_along_col(std::size_t i_col) noexcept -> GridIteratorPair<T>
    {
        const auto begin = static_cast<T*>(&data_[0] + i_col);
        const auto end = static_cast<T*>(begin + n_rows_ * n_cols_);
        const auto stride = std::size_t {n_cols_};

        return {
            GridIterator<T> {begin, stride},
             GridIterator<T> {end,   stride}
        };
    }

    constexpr auto iterator_along_col(std::size_t i_col) const noexcept -> ConstGridIteratorPair<T>
    {
        const auto begin = static_cast<const T*>(&data_[0] + i_col);
        const auto end = static_cast<const T*>(begin + n_rows_ * n_cols_);
        const auto stride = std::size_t {n_cols_};

        return {
            ConstGridIterator<T> {begin, stride},
             ConstGridIterator<T> {end,   stride}
        };
    }

    constexpr auto iterator_along_row(std::size_t i_row) noexcept -> GridIteratorPair<T>
    {
        const auto begin = static_cast<T*>(&data_[0] + i_row * n_cols_);
        const auto end = static_cast<T*>(begin + n_cols_);
        const auto stride = std::size_t {1};

        return {
            GridIterator<T> {begin, stride},
             GridIterator<T> {end,   stride}
        };
    }

    constexpr auto iterator_along_row(std::size_t i_row) const noexcept -> ConstGridIteratorPair<T>
    {
        const auto begin = static_cast<const T*>(&data_[0] + i_row * n_cols_);
        const auto end = static_cast<const T*>(begin + n_cols_);
        const auto stride = std::size_t {1};

        return {
            ConstGridIterator<T> {begin, stride},
             ConstGridIterator<T> {end,   stride}
        };
    }

private:
    std::size_t n_rows_;
    std::size_t n_cols_;
    std::vector<T> data_;
};

}  // namespace mathtools
