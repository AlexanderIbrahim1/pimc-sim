#pragma once

#include <cmath>
#include <concepts>
#include <cstddef>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <geometries/attard/three_body.hpp>
#include <interactions/three_body/potential_concepts.hpp>

namespace interact
{

template <typename Potential, std::floating_point FP, std::size_t NDIM>
requires TripletPotential<Potential>
class ThreeBodyPointPotential
{
    using Point = coord::Cartesian<FP, NDIM>;

public:
    explicit ThreeBodyPointPotential(Potential pot)
        : pot_ {std::move(pot)}
    {}

    constexpr auto operator()(const Point& p0, const Point& p1, const Point& p2) const noexcept -> FP
    {
        return pot_(coord::distance(p0, p1), coord::distance(p0, p2), coord::distance(p1, p2));
    }

private:
    Potential pot_;
};

template <typename Potential, std::floating_point FP, std::size_t NDIM>
requires TripletPotential<Potential>
class PeriodicThreeBodyPointPotential
{
    using Point = coord::Cartesian<FP, NDIM>;
    using Box = coord::BoxSides<FP, NDIM>;

public:
    explicit PeriodicThreeBodyPointPotential(Potential pot, Box box)
        : cutoff_dist_sq_ {coord::box_cutoff_distance_squared(box)}
        , box_ {std::move(box)}
        , pot_ {std::move(pot)}
    {}

    auto operator()(const Point& p0, const Point& p1, const Point& p2) const noexcept -> FP
    {
        const auto dist01 = coord::distance_periodic(p0, p1, box_);
        const auto dist02 = coord::distance_periodic(p0, p2, box_);
        const auto dist12 = coord::distance_periodic(p1, p2, box_);
        return pot_(dist01, dist02, dist12);
    }

    auto within_box_cutoff(const Point& p0, const Point& p1, const Point& p2) const noexcept -> FP
    {
        const auto [dist01_sq, dist02_sq, dist12_sq] = geom::three_body_attard_side_lengths_squared({p0, p1, p2}, box_);

        if (dist01_sq < cutoff_dist_sq_ && dist02_sq < cutoff_dist_sq_ && dist12_sq < cutoff_dist_sq_) {
            return pot_(std::sqrt(dist01_sq), std::sqrt(dist02_sq), std::sqrt(dist12_sq));
        }
        else {
            return FP {0.0};
        }
    }

    auto within_box_cutoff_incorrect(const Point& p0, const Point& p1, const Point& p2) const noexcept -> FP
    {
        const auto dist01_sq = coord::distance_squared_periodic(p0, p1, box_);
        const auto dist02_sq = coord::distance_squared_periodic(p0, p2, box_);
        const auto dist12_sq = coord::distance_squared_periodic(p1, p2, box_);

        if (dist01_sq < cutoff_dist_sq_ && dist02_sq < cutoff_dist_sq_ && dist12_sq < cutoff_dist_sq_) {
            return pot_(std::sqrt(dist01_sq), std::sqrt(dist02_sq), std::sqrt(dist12_sq));
        }
        else {
            return FP {0.0};
        }
    }

private:
    FP cutoff_dist_sq_;
    Box box_;
    Potential pot_;
};

}  // namespace interact
