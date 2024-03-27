#pragma once

#include <concepts>
#include <utility>

#include <mathtools/interpolate/trilinear_interp.hpp>
#include <interactions/three_body/three_body_pointwise.hpp>

namespace interact
{

/*
The isotropic three-body potential energy surface for parahydrogen
published in "J. Chem. Phys. 156, 044301 (2022)"
*/
template <std::floating_point FP>
class ThreeBodyParaH2Potential
{
public:
    ThreeBodyParaH2Potential(mathtools::TrilinearInterpolator<FP> interpolator, FP c9_coefficient)
        : interpolator_ {std::move(interpolator)}
        , atm_potential_ {c9_coefficient}
    {}

private:
    mathtools::TrilinearInterpolator<FP> interpolator_;
    AxilrodTellerMuto<FP> atm_potential_;
};

}  // namespace interact
