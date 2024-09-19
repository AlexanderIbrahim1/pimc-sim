#pragma once

#include <concepts>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <common/common_utils.hpp>
#include <mathtools/mathtools_utils.hpp>

namespace mathtools
{

// PLAN: modify the Grid3D constructors to use the Shape3D instance instead
struct Shape3D
{
    std::size_t size0;
    std::size_t size1;
    std::size_t size2;
};

struct Index3D
{
    std::size_t idx0;
    std::size_t idx1;
    std::size_t idx2;
};

template <common_utils::Numeric Number>
class Grid3D
{
public:
    Grid3D(Shape3D shape)
        : shape_ {shape}
        , data_ {}
    {
        mathtools_utils::ctr_check_positive(shape_.size0, "size0");
        mathtools_utils::ctr_check_positive(shape_.size1, "size1");
        mathtools_utils::ctr_check_positive(shape_.size2, "size2");

        data_.resize(shape_.size0 * shape_.size1 * shape_.size2);
    }

    Grid3D(std::vector<Number> data, Shape3D shape)
        : shape_ {shape}
        , data_ {std::move(data)}
    {
        mathtools_utils::ctr_check_positive(shape_.size0, "size0");
        mathtools_utils::ctr_check_positive(shape_.size1, "size1");
        mathtools_utils::ctr_check_positive(shape_.size2, "size2");

        if (shape_.size0 * shape_.size1 * shape_.size2 != data_.size()) {
            auto err_msg = std::stringstream {};
            err_msg << "Attempting to create a Grid3D instance with invalid side lengths provided.\n";
            err_msg << "data size: " << data_.size() << '\n';
            err_msg << "side lengths provided: ";
            err_msg << shape_.size0 << ", " << shape_.size1 << ", " << shape_.size2 << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }

    constexpr auto get(std::size_t i0, std::size_t i1, std::size_t i2) const noexcept -> Number
    {
        const auto index = i2 + shape_.size2 * i1 + shape_.size2 * shape_.size1 * i0;
        return data_[index];
    }

    constexpr void set(std::size_t i0, std::size_t i1, std::size_t i2, Number value) noexcept
    {
        const auto index = i2 + shape_.size2 * i1 + shape_.size2 * shape_.size1 * i0;
        data_[index] = value;
    }

    constexpr auto data() const noexcept -> const std::vector<Number>&
    {
        return data_;
    }

    constexpr auto shape() const noexcept -> Shape3D
    {
        return shape_;
    }

private:
    Shape3D shape_;
    std::vector<Number> data_;
};

}  // namespace mathtools
