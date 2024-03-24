#pragma once

#include <concepts>
#include <cstddef>
#include <vector>
#include <sstring>
#include <stdexcept>
#include <tuple>

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

template <common_utils::Numeric Number>
class Grid3D
{
public:
    Grid3D(std::size_t size0, std::size_t size1, std::size_t size2)
        : size0_ {size0}
        , size1_ {size1}
        , size2_ {size2}
        , data_ {}
    {
        mathtools_utils::ctr_check_positive(size0_, "size0");
        mathtools_utils::ctr_check_positive(size1_, "size1");
        mathtools_utils::ctr_check_positive(size2_, "size1");

        data_.resize(size0_ * size1_ * size2_);
    }

    Grid3D(std::vector<Number> data, std::size_t size0, std::size_t size1, std::size_t size2)
        : size0_ {size0}
        , size1_ {size1}
        , size2_ {size2}
        , data_ {std::move(data)}
    {
        mathtools_utils::ctr_check_positive(size0_, "size0");
        mathtools_utils::ctr_check_positive(size1_, "size1");
        mathtools_utils::ctr_check_positive(size2_, "size1");

        if (size0_ * size1_ * size2_ != data_.size()) {
            auto err_msg = std::stringstream {};
            err_msg << "Attempting to create a Grid3D instance with invalid side lengths provided.\n";
            err_msg << "data size: " << data_.size() << '\n';
            err_msg << "side lengths provided: ";
            err_msg << size0_ << ", " << size1_ << ", " << size2_ << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }

    constexpr auto get(std::size_t i0, std::size_t i1, std::size_t i2) const noexcept -> Number
    {
        const auto index = i0 * size1_ * size2_ + i1 * size2_ + i2;
        return data_[index];
    }

    constexpr void set(std::size_t i0, std::size_t i1, std::size_t i2, Number value) const noexcept
    {
        const auto index = i0 * size1_ * size2_ + i1 * size2_ + i2;
        data_[index] = value;
    }

    constexpr auto data() const noexcept -> const std::vector<Number>&
    {
        return data_;
    }

    constexpr auto shape() const noexcept
    {
        return {size0_, size1_, size2_};
    }

private:
    std::size_t size0_;
    std::size_t size1_;
    std::size_t size2_;
    std::vector<Number> data_;
};

}  // namespace mathtools
