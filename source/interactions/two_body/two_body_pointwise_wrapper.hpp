#pragma once

#include <concepts>
#include <cstddef>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>

namespace interact
{

template <typename Potential, typename FP, std::size_t NDIM>
concept PairPointPotential = requires(Potential pot) {
    requires std::is_floating_point_v<FP>;
    {
        pot(coord::Cartesian<FP, NDIM> {}, coord::Cartesian<FP, NDIM> {})
    } -> std::same_as<FP>;
};

template <typename Potential, std::floating_point FP, std::size_t NDIM>
class PointwisePairPotential
{
    static_assert(PairDistancePotential<Potential>);
    using Point = coord::Cartesian<FP, NDIM>;

public:
    explicit PointwisePairPotential(Potential pot)
        : pot_ {std::move(pot)}
    {}

    constexpr auto operator()(const Point& p0, const Point& p1) const noexcept -> FP
    {
        return pot_(coord::distance(p0, p1));
    }

private:
    Potential pot_;
};

template <typename Potential, std::floating_point FP, std::size_t NDIM>
class PeriodicPointwisePairPotential
{
    static_assert(PairDistancePotential<Potential>);
    using Point = coord::Cartesian<FP, NDIM>;
    using Box = coord::BoxSides<FP, NDIM>;

public:
    explicit PeriodicPointwisePairPotential(Potential pot, Box box)
        : pot_ {std::move(pot)}
        , box_ {std::move(box)}
    {}

    constexpr auto operator()(const Point& p0, const Point& p1) const noexcept -> FP
    {
        return pot_(coord::distance_periodic(p0, p1, box_));
    }

private:
    Potential pot_;
    Box box_;
};

}  // namespace interact
