#pragma once

#include <concepts>

#include <mathtools/grid/grid3d.hpp>
#include <mathtools/mathtools_utils.hpp>

namespace mathtools
{

template <std::floating_point FP>
class RegularTrilinearInterpolator
{
public:
    RegularTrilinearInterpolator(
        Grid3D<FP> grid,
        mathtools_utils::AxisLimits<FP> limits0,
        mathtools_utils::AxisLimits<FP> limits1,
        mathtools_utils::AxisLimits<FP> limits2
    )
        : grid_ {std::move(grid)}
        , limits0_ {limits0}
        , limits1_ {limits1}
        , limits2_ {limits2}
    {}

private:
    Grid3D<FP> grid_;
    mathtools_utils::AxisLimits<FP> limits0_;
    mathtools_utils::AxisLimits<FP> limits1_;
    mathtools_utils::AxisLimits<FP> limits2_;
};

}  // namespace mathtools
