#pragma once

#include <concepts>
#include <cstddef>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <interactions/two_body/potential_concepts.hpp>

namespace interact
{

template <typename Potential, std::floating_point FP, std::size_t NDIM>
requires PairPotential<Potential>
class PairDistancePotential
{
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
requires PairPotential<Potential>
class PeriodicPairDistancePotential
{
    using Point = coord::Cartesian<FP, NDIM>;
    using Box = coord::BoxSides<FP, NDIM>;

public:
    explicit PeriodicPairDistancePotential(Potential pot, Box box)
        : cutoff_distance_ {coord::box_cutoff_distance(box)}
        , pot_ {std::move(pot)}
        , box_ {std::move(box)}
    {}

    constexpr auto operator()(const Point& p0, const Point& p1) const noexcept -> FP
    {
        return pot_(coord::distance_periodic(p0, p1, box_));
    }

    constexpr auto within_box_cutoff(const Point& p0, const Point& p1) const noexcept -> FP
    {
        const auto distance = coord::distance_periodic(p0, p1, box_);

        if (distance < cutoff_distance_) {
            return pot_(distance);
        }
        else {
            return FP {0.0};
        }
    }

private:
    FP cutoff_distance_;
    Potential pot_;
    Box box_;
};

template <typename Potential, std::floating_point FP, std::size_t NDIM>
requires PairPotential<Potential>
class PeriodicPairDistanceSquaredPotential
{
    using Point = coord::Cartesian<FP, NDIM>;
    using Box = coord::BoxSides<FP, NDIM>;

public:
    explicit PeriodicPairDistanceSquaredPotential(Potential pot, Box box)
        : cutoff_distance_squared_ {coord::box_cutoff_distance_squared(box)}
        , pot_ {std::move(pot)}
        , box_ {std::move(box)}
    {}

    constexpr auto operator()(const Point& p0, const Point& p1) const noexcept -> FP
    {
        return pot_(coord::distance_squared_periodic(p0, p1, box_));
    }

    constexpr auto within_box_cutoff(const Point& p0, const Point& p1) const noexcept -> FP
    {
        const auto distance_squared = coord::distance_squared_periodic(p0, p1, box_);

        if (distance_squared < cutoff_distance_squared_) {
            return pot_(distance_squared);
        }
        else {
            return FP {0.0};
        }
    }

private:
    FP cutoff_distance_squared_;
    Potential pot_;
    Box box_;
};

}  // namespace interact
