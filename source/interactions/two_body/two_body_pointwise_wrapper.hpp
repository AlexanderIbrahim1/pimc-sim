#pragma once

#include <concepts>
#include <cstddef>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <interactions/two_body/potential_concepts.hpp>

namespace interact
{

template <typename Potential, std::floating_point FP, std::size_t NDIM>
class PairDistancePotential
{
    static_assert(PairPotential<Potential>);
    using Point = coord::Cartesian<FP, NDIM>;

public:
    explicit PairDistancePotential(Potential pot)
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
class PeriodicPairDistancePotential
{
    static_assert(PairPotential<Potential>);
    using Point = coord::Cartesian<FP, NDIM>;
    using Box = coord::BoxSides<FP, NDIM>;

public:
    explicit PeriodicPairDistancePotential(Potential pot, Box box)
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

template <typename Potential, std::floating_point FP, std::size_t NDIM>
class PeriodicPairDistanceSquaredPotential
{
    static_assert(PairPotential<Potential>);
    using Point = coord::Cartesian<FP, NDIM>;
    using Box = coord::BoxSides<FP, NDIM>;

public:
    explicit PeriodicPairDistanceSquaredPotential(Potential pot, Box box)
        : pot_ {std::move(pot)}
        , box_ {std::move(box)}
    {}

    constexpr auto operator()(const Point& p0, const Point& p1) const noexcept -> FP
    {
        return pot_(coord::distance_squared_periodic(p0, p1, box_));
    }

private:
    Potential pot_;
    Box box_;
};

}  // namespace interact
