#pragma once

#include <concepts>
#include <initializer_list>
#include <span>
#include <vector>

#include <coordinates/coordinates.hpp>
#include <mathtools/grid/grid2d.hpp>
#include <mathtools/grid/grid_iterator.hpp>

namespace worldline
{

template <std::floating_point FP, std::size_t NDIM>
class Worldlines
{
public:
    using Point = coord::Cartesian<FP, NDIM>;

    explicit Worldlines(mathtools::Grid2D<Point> coordinates)
        : coordinates_ {std::move(coordinates)}
    {}

    /*
        For performance reasons, we want all the beads on the same imaginary time step
        to be contiguous in memory.

        So `n_particles_v`, should be the number of columns in the grid. This allows the
        particles to be next to each other in memory.

        This leaves `n_timeslices_v` as the number of rows.
    */
    explicit Worldlines(std::size_t n_timeslices_v, std::size_t n_particles_v)
        : coordinates_ {n_timeslices_v, n_particles_v}
    {}

    constexpr auto n_worldlines() const noexcept -> std::size_t
    {
        return coordinates_.n_cols();
    }

    constexpr auto n_timeslices() const noexcept -> std::size_t
    {
        return coordinates_.n_rows();
    }

    constexpr auto get(std::size_t i_timeslice, std::size_t i_worldline) const noexcept -> const Point&
    {
        return coordinates_.get(i_timeslice, i_worldline);
    }

    constexpr void set(std::size_t i_timeslice, std::size_t i_worldline, Point point) noexcept
    {
        coordinates_.set(i_timeslice, i_worldline, std::move(point));
    }

    constexpr auto timeslice(std::size_t i_timeslice) noexcept -> std::span<Point>
    {
        return coordinates_.iterator_along_row(i_timeslice);
    }

    constexpr auto timeslice(std::size_t i_timeslice) const noexcept -> std::span<const Point>
    {
        return coordinates_.iterator_along_row(i_timeslice);
    }

    constexpr auto worldline(std::size_t i_worldline) noexcept -> mathtools::GridIteratorPair<Point>
    {
        return coordinates_.iterator_along_col(i_worldline);
    }

    constexpr auto worldline(std::size_t i_worldline) const noexcept -> mathtools::ConstGridIteratorPair<Point>
    {
        return coordinates_.iterator_along_col(i_worldline);
    }

private:
    mathtools::Grid2D<Point> coordinates_ {};
};

template <std::floating_point FP, std::size_t NDIM>
constexpr auto worldlines_from_positions(
    const std::vector<coord::Cartesian<FP, NDIM>>& points,
    std::size_t n_timeslices
) noexcept -> Worldlines<FP, NDIM>
{
    const auto n_particles = points.size();
    auto worldlines = Worldlines<FP, NDIM> {n_timeslices, n_particles};

    for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
        for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
            worldlines.set(i_tslice, i_part, points[i_part]);
        }
    }

    return worldlines;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto calculate_centroid(const Worldlines<FP, NDIM>& worldlines, std::size_t i_particle) -> coord::Cartesian<FP, NDIM>
{
    auto accumulated_centroid = coord::Cartesian<FP, NDIM>::origin();
    for (const auto& bead : worldlines.worldline(i_particle)) {
        accumulated_centroid += bead;
    }
    accumulated_centroid /= static_cast<FP>(worldlines.n_timeslices());

    return accumulated_centroid;
}

template <std::floating_point FP, std::size_t NDIM>
constexpr auto calculate_all_centroids(const Worldlines<FP, NDIM>& worldlines)
    -> std::vector<coord::Cartesian<FP, NDIM>>
{
    auto centroids = std::vector<coord::Cartesian<FP, NDIM>> {};
    centroids.reserve(worldlines.n_worldlines());
    for (std::size_t i_part {0}; i_part < worldlines.n_worldlines(); ++i_part) {
        centroids.push_back(calculate_centroid(worldlines, i_part));
    }

    return centroids;
}

// ----------------------------------------------------------------------------
// --- DELETE BELOW AFTER REFACTORING IS DONE
// ----------------------------------------------------------------------------

// template <std::floating_point FP, std::size_t NDIM>
// class Worldline
// {
// public:
//     using Point = coord::Cartesian<FP, NDIM>;
// 
//     constexpr Worldline() = default;
// 
//     constexpr Worldline(std::initializer_list<Point> ilist)
//         : points_ {ilist}
//     {}
// 
//     constexpr Worldline(std::vector<Point> points)
//         : points_ {std::move(points)}
//     {}
// 
//     constexpr Worldline(const std::span<const Point> container)
//         : points_ {std::begin(container), std::end(container)}
//     {}
// 
//     constexpr auto points() const noexcept -> const std::vector<Point>&
//     {
//         return points_;
//     }
// 
//     constexpr auto operator[](std::size_t index) const noexcept -> Point
//     {
//         // non-modifying access with bounds checking on during debug mode only
//         assert(index < points_.size());
//         return points_[index];
//     }
// 
//     constexpr auto operator[](std::size_t index) noexcept -> Point&
//     {
//         // modifying access with bounds checking on during debug mode only
//         assert(index < points_.size());
//         return points_[index];
//     }
// 
//     constexpr auto size() const noexcept -> std::size_t
//     {
//         return points_.size();
//     }
// 
// private:
//     std::vector<Point> points_ {};
// };
// 
// template <std::floating_point FP, std::size_t NDIM>
// constexpr auto worldlines_from_positions(
//     const std::vector<coord::Cartesian<FP, NDIM>>& points,
//     std::size_t n_timeslices
// ) noexcept -> std::vector<Worldline<FP, NDIM>>
// {
//     auto worldlines = std::vector<Worldline<FP, NDIM>> {};
//     worldlines.reserve(n_timeslices);
// 
//     for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
//         worldlines.push_back(Worldline<FP, NDIM> {points});
//     }
// 
//     return worldlines;
// }
// 
// template <std::floating_point FP, std::size_t NDIM>
// constexpr auto calculate_centroid(const std::vector<Worldline<FP, NDIM>>& worldlines, std::size_t i_particle)
//     -> coord::Cartesian<FP, NDIM>
// {
//     const auto n_timeslices = worldlines.size();
// 
//     auto accumulated_centroid = coord::Cartesian<FP, NDIM>::origin();
//     for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
//         accumulated_centroid += worldlines[i_tslice][i_particle];
//     }
// 
//     accumulated_centroid /= static_cast<FP>(n_timeslices);
// 
//     return accumulated_centroid;
// }
// 
// template <std::floating_point FP, std::size_t NDIM>
// constexpr auto calculate_all_centroids(const std::vector<Worldline<FP, NDIM>>& worldlines)
//     -> std::vector<coord::Cartesian<FP, NDIM>>
// {
//     using Point = coord::Cartesian<FP, NDIM>;
// 
//     const auto n_particles = worldlines[0].size();
// 
//     auto centroids = std::vector<Point> {};
//     centroids.reserve(n_particles);
//     for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
//         centroids.push_back(calculate_centroid(worldlines, i_part));
//     }
// 
//     return centroids;
// }

}  // namespace worldline
