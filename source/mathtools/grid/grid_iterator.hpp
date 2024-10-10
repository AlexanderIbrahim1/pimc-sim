#pragma once

#include <cstddef>

/*
    GridIterator and ConstGridIterator iterate over the elements of grid classes like
    Grid2D and Grid3D, accounting for the stride.

    The only difference between GridIterator and ConstGridIterator are:
      + GridIterator:
        - `ptr_` is of type `iterator`
        - `operator*()` returns `reference`
      + ConstGridIterator:
        - `ptr_` is of type `const_iterator`
        - `operator*()` returns `const_reference`
*/

namespace mathtools
{

template <typename T>
class GridIterator
{
public:
    // clang-format off
    using value_type             = T;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using reference              = value_type&;
    using const_reference        = const value_type&;                    
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    // clang-format on

    constexpr GridIterator(iterator ptr, std::size_t stride)
        : ptr_ {ptr}
        , stride_ {stride}
    {}

    constexpr auto operator*() const noexcept -> reference
    {
        return *ptr_;
    }

    constexpr auto operator->() const noexcept -> iterator
    {
        return ptr_;
    }

    constexpr auto operator++() noexcept -> GridIterator&
    {
        ptr_ += stride_;
        return *this;
    }

    constexpr auto operator++(int) noexcept -> GridIterator
    {
        GridIterator temp = *this;
        ++(*this);
        return temp;
    }

    template <typename U>
    friend constexpr auto operator==(const GridIterator<U>& left, const GridIterator<U>& right) noexcept -> bool
    {
        return left.ptr_ == right.ptr_;
    }

    template <typename U>
    friend constexpr auto operator!=(const GridIterator<U>& left, const GridIterator<U>& right) noexcept -> bool
    {
        return !(left == right);
    }

private:
    iterator ptr_;
    std::size_t stride_;
};

template <typename T>
class GridIteratorPair
{
public:
    GridIteratorPair(GridIterator<T> begin_v, GridIterator<T> end_v)
        : begin_ {std::move(begin_v)}
        , end_ {std::move(end_v)}
    {}

    constexpr auto begin() noexcept -> GridIterator<T>
    {
        return begin_;
    }

    constexpr auto end() noexcept -> GridIterator<T>
    {
        return end_;
    }

    constexpr auto begin() const noexcept -> GridIterator<T>
    {
        return begin_;
    }

    constexpr auto end() const noexcept -> GridIterator<T>
    {
        return end_;
    }

private:
    GridIterator<T> begin_;
    GridIterator<T> end_;
};

template <typename T>
class ConstGridIterator
{
public:
    // clang-format off
    using value_type             = T;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using reference              = value_type&;
    using const_reference        = const value_type&;                    
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    // clang-format on

    constexpr ConstGridIterator(const_iterator ptr, std::size_t stride)
        : ptr_ {ptr}
        , stride_ {stride}
    {}

    constexpr auto operator*() const noexcept -> const_reference
    {
        return *ptr_;
    }

    constexpr auto operator->() const noexcept -> const_iterator
    {
        return ptr_;
    }

    constexpr auto operator++() noexcept -> ConstGridIterator&
    {
        ptr_ += stride_;
        return *this;
    }

    constexpr auto operator++(int) noexcept -> ConstGridIterator
    {
        ConstGridIterator temp = *this;
        ++(*this);
        return temp;
    }

    template <typename U>
    friend constexpr auto operator==(const ConstGridIterator<U>& left, const ConstGridIterator<U>& right) noexcept
        -> bool
    {
        return left.ptr_ == right.ptr_;
    }

    template <typename U>
    friend constexpr auto operator!=(const ConstGridIterator<U>& left, const ConstGridIterator<U>& right) noexcept
        -> bool
    {
        return !(left == right);
    }

private:
    const_iterator ptr_;
    std::size_t stride_;
};

template <typename T>
class ConstGridIteratorPair
{
public:
    ConstGridIteratorPair(ConstGridIterator<T> begin_v, ConstGridIterator<T> end_v)
        : begin_ {std::move(begin_v)}
        , end_ {std::move(end_v)}
    {}

    constexpr auto begin() noexcept -> ConstGridIterator<T>
    {
        return begin_;
    }

    constexpr auto end() noexcept -> ConstGridIterator<T>
    {
        return end_;
    }

    constexpr auto begin() const noexcept -> ConstGridIterator<T>
    {
        return begin_;
    }

    constexpr auto end() const noexcept -> ConstGridIterator<T>
    {
        return end_;
    }

private:
    ConstGridIterator<T> begin_;
    ConstGridIterator<T> end_;
};

}  // namespace mathtools
