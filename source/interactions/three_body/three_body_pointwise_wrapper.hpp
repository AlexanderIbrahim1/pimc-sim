#pragma once

#include <cmath>
#include <concepts>
#include <cstddef>

#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <interactions/three_body/potential_concepts.hpp>

namespace interact
{

template <typename Potential, std::floating_point FP, std::size_t NDIM>
requires TripletPotential<Potential>
class TripletDistancePotential
{
    using Point = coord::Cartesian<FP, NDIM>;

public:
    explicit TripletDistancePotential(Potential pot)
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
class PeriodicTripletDistancePotential
{
    using Point = coord::Cartesian<FP, NDIM>;
    using Box = coord::BoxSides<FP, NDIM>;

public:
    explicit PeriodicTripletDistancePotential(Potential pot, Box box)
        : cutoff_dist_sq_ {coord::box_cutoff_distance_squared(box)}
        , pot_ {std::move(pot)}
    {}

    auto operator()(const Point& p0, const Point& p1, const Point& p2) const noexcept -> FP
    {
        return pot_(coord::distance(p0, p1), coord::distance(p0, p2), coord::distance(p1, p2));
    }

    auto within_box_cutoff(const Point& p0, const Point& p1, const Point& p2) const noexcept -> FP
    {
        // I am 99% sure what I just wrote below is wrong, and I need to reimplement what was put in
        // the Attard paper, or just replicate what I put in the pimcanalysis Python repo (where I know
        // I implemented it correctly, for both the 3B and 4B interactions)

        // // NOTE: it doesn't matter which of the points that the group of three points is centred around;
        // // whether or not the Attard minimage condition is accepted or rejected does not change; thus, we
        // // can arbitrarily choose `p0`

        // // p0 becomes the origin
        // const auto p01 = p1 - p0;
        // const auto p02 = p2 - p0;

        // const auto dist01_sq = coord::norm_squared(p01);
        // const auto dist02_sq = coord::norm_squared(p02);
        // const auto dist12_sq = coord::distance_squared(p01, p02);

        // if (dist01_sq < cutoff_dist_sq_ && dist02_sq < cutoff_dist_sq_ && dist12_sq < cutoff_dist_sq_) {
        //     return pot_(std::sqrt(dist01_sq), std::sqrt(dist02_sq), std::sqrt(dist12_sq));
        // }
        // else {
        //     return FP {0.0};
        // }
    }

private:
    FP cutoff_dist_sq_;
    Potential pot_;
};

}  // namespace interact
