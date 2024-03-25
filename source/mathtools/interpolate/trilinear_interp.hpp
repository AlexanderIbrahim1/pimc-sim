#pragma once

#include <concepts>

#include <mathtools/grid/grid3d.hpp>
#include <mathtools/mathtools_utils.hpp>

namespace mathtools
{

template <std::floating_point FP>
class TrilinearInterpolator
{
public:
    TrilinearInterpolator(
        Grid3D<FP> grid,
        mathtools_utils::AxisLimits<FP> limits0,
        mathtools_utils::AxisLimits<FP> limits1,
        mathtools_utils::AxisLimits<FP> limits2
    )
        : grid_ {std::move(grid)}
        , limits0_ {limits0}
        , limits1_ {limits1}
        , limits2_ {limits2}
    {
        const auto shape = grid_.shape();

        mathtools_utils::ctr_check_data_size_at_least_two(shape.size0);
        mathtools_utils::ctr_check_data_size_at_least_two(shape.size1);
        mathtools_utils::ctr_check_data_size_at_least_two(shape.size2);

        d0_ = (limits0_.upper() - limits0_.lower()) / static_cast<FP>(shape.size0 - 1);
        d1_ = (limits1_.upper() - limits1_.lower()) / static_cast<FP>(shape.size1 - 1);
        d2_ = (limits2_.upper() - limits2_.lower()) / static_cast<FP>(shape.size2 - 1);
    }

    constexpr auto operator()(FP x0, FP x1, FP x2) const noexcept -> FP
    {
        const auto idx = lower_indices_(x0, x1, x2);
        return interpolate_(x0, x1, x2, idx);
    }

    auto get_checked(FP x0, FP x1, FP x2) const -> FP
    {
        mathtools_utils::is_in_halfopen_limits(limits0_, x0, "x");
        mathtools_utils::is_in_halfopen_limits(limits1_, x1, "y");
        mathtools_utils::is_in_halfopen_limits(limits2_, x2, "z");

        const auto idx = lower_indices_(x0, x1, x2);
        return interpolate_(x0, x1, x2, idx);
    }

private:
    Grid3D<FP> grid_;
    mathtools_utils::AxisLimits<FP> limits0_;
    mathtools_utils::AxisLimits<FP> limits1_;
    mathtools_utils::AxisLimits<FP> limits2_;
    FP d0_;
    FP d1_;
    FP d2_;

    constexpr auto lower_indices_(FP x0, FP x1, FP x2) const noexcept -> Index3D {
        const auto idx0 = static_cast<std::size_t>((x0 - limits0_.lower()) / d0_);
        const auto idx1 = static_cast<std::size_t>((x1 - limits1_.lower()) / d1_);
        const auto idx2 = static_cast<std::size_t>((x2 - limits2_.lower()) / d2_);

        return {idx0, idx1, idx2};
    }

    constexpr auto interpolate_(FP x0, FP x1, FP x2, Index3D idx) const noexcept -> FP {
        const auto [idx0, idx1, idx2] = idx;

        const auto left0 = limits0_.lower() + static_cast<FP>(idx0) * d0_;
        const auto left1 = limits1_.lower() + static_cast<FP>(idx1) * d1_;
        const auto left2 = limits2_.lower() + static_cast<FP>(idx2) * d2_;

        const auto diff0 = (x0 - left0) / d0_;
        const auto diff1 = (x1 - left1) / d1_;
        const auto diff2 = (x2 - left2) / d2_;

        const auto mdiff0 = 1.0 - diff0;
        const auto mdiff1 = 1.0 - diff1;
        const auto mdiff2 = 1.0 - diff2;

        // clang-format off
        const auto e000 = grid_.get(idx0    , idx1    , idx2    );
        const auto e001 = grid_.get(idx0    , idx1    , idx2 + 1);
        const auto e010 = grid_.get(idx0    , idx1 + 1, idx2    );
        const auto e011 = grid_.get(idx0    , idx1 + 1, idx2 + 1);
        const auto e100 = grid_.get(idx0 + 1, idx1    , idx2    );
        const auto e101 = grid_.get(idx0 + 1, idx1    , idx2 + 1);
        const auto e110 = grid_.get(idx0 + 1, idx1 + 1, idx2    );
        const auto e111 = grid_.get(idx0 + 1, idx1 + 1, idx2 + 1);

        const auto f00 = mdiff0 * e000 + diff0 * e100;
        const auto f01 = mdiff0 * e001 + diff0 * e101;
        const auto f10 = mdiff0 * e010 + diff0 * e110;
        const auto f11 = mdiff0 * e011 + diff0 * e111;

        const auto g0  = mdiff1 * f00 + diff1 * f10;
        const auto g1  = mdiff1 * f01 + diff1 * f11;

        const auto h   = mdiff2 * g0 + diff2 * g1;
        // clang-format on

        return h;
    }
};

}  // namespace mathtools
