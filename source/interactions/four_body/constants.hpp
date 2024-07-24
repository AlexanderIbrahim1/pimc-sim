#pragma once

/*
    This header file contains constants used in the various components that make up the
    four-body interaction potential.

    The reason the constants are in a header file instead of an editable text file, is that
    the values here, for the most part, cannot be modified by the user without essentially
    breaking the validity of the model.
*/

#include <concepts>

namespace interact
{

namespace constants4b
{

template <std::floating_point FP>
constexpr FP MIN_SIDELENGTH = FP {2.2};

template <std::floating_point FP>
constexpr FP MAX_SIDELENGTH = FP {4.5};

template <std::floating_point FP>
constexpr FP STANDARDIZE_FROM_LEFT = FP {0.0};

template <std::floating_point FP>
constexpr FP STANDARDIZE_FROM_RIGHT = FP {1.0} / MIN_SIDELENGTH<FP>;

template <std::floating_point FP>
constexpr FP STANDARDIZE_TO_LEFT = FP {0.0};

template <std::floating_point FP>
constexpr FP STANDARDIZE_TO_RIGHT = FP {1.0};

template <std::floating_point FP>
constexpr FP BADE_COEFF_MIDZUNO_KIHARA = FP {33760.08799487081};

template <std::floating_point FP>
constexpr FP BADE_COEFF_AVDZ = FP {31666.96253918882};

template <std::floating_point FP>
constexpr FP BADE_COEFF_AVTZ = FP {29492.81287231914};

template <std::floating_point FP>
constexpr FP SHORT_RANGE_CORRECT_SLOPE_MIN = FP {6.0};

template <std::floating_point FP>
constexpr FP SHORT_RANGE_CORRECT_SLOPE_MAX = FP {8.0};

template <std::floating_point FP>
constexpr FP RESCALING_EXPON_COEFF = FP {3180260.750000};

template <std::floating_point FP>
constexpr FP RESCALING_EXPON_DECAY = FP {4.623057};

template <std::floating_point FP>
constexpr FP RESCALING_DISP_COEFF = FP {4220.011};

template <std::floating_point FP>
constexpr FP REVERSE_RESCALING_LIMITS_TO_LEFT = FP {-1.0};

template <std::floating_point FP>
constexpr FP REVERSE_RESCALING_LIMITS_TO_RIGHT = FP {1.0};

template <std::floating_point FP>
constexpr FP REVERSE_RESCALING_LIMITS_FROM_LEFT = FP {-3.2619903087615967};

template <std::floating_point FP>
constexpr FP REVERSE_RESCALING_LIMITS_FROM_RIGHT = FP {8.64592170715332};

template <std::floating_point FP>
constexpr FP SHORT_RANGE_SCALING_STEP = FP {0.01};

template <std::floating_point FP>
constexpr FP LOWER_SHORT_DISTANCE = FP {2.2};

template <std::floating_point FP>
constexpr FP UPPER_SHORT_DISTANCE = FP {2.25};

template <std::floating_point FP>
constexpr FP LOWER_MIXED_DISTANCE = FP {4.0};

template <std::floating_point FP>
constexpr FP UPPER_MIXED_DISTANCE = FP {4.5};

}  // namespace constants4b

}  // namespace interact
