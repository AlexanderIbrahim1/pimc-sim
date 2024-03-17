#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>

namespace coord
{

template <std::floating_point FP, std::size_t NDIM, bool IsSquaredDistance>
class PeriodicMeasureWrapperBase_
{
public:
    explicit PeriodicMeasureWrapperBase_(BoxSides<FP, NDIM> box_sides)
        : box_sides_ {std::move(box_sides)}
    {}

    constexpr auto operator()(const Cartesian<FP, NDIM>& point0, const Cartesian<FP, NDIM>& point1) const noexcept -> FP
    {
        if constexpr (IsSquaredDistance) {
            return distance_squared_periodic(point0, point1, box_sides_);
        }
        else {
            return distance_periodic(point0, point1, box_sides_);
        }
    }

private:
    BoxSides<FP, NDIM> box_sides_;
};

template <std::floating_point FP, std::size_t NDIM>
class PeriodicDistanceMeasureWrapper : public PeriodicMeasureWrapperBase_<FP, NDIM, false>
{
public:
    using PeriodicMeasureWrapperBase_<FP, NDIM, false>::PeriodicMeasureWrapperBase_;
};

template <std::floating_point FP, std::size_t NDIM>
class PeriodicDistanceSquaredMeasureWrapper : public PeriodicMeasureWrapperBase_<FP, NDIM, true>
{
public:
    using PeriodicMeasureWrapperBase_<FP, NDIM, true>::PeriodicMeasureWrapperBase_;
};

}  // namespace coord
