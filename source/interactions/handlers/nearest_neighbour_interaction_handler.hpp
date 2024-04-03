#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <utility>
#include <vector>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <interactions/two_body/potential_concepts.hpp>
#include <mathtools/grid/square_adjacency_matrix.hpp>
#include <worldline/worldline.hpp>

namespace interact
{

template <std::floating_point FP, std::size_t NDIM>
void update_centroid_adjacency_matrix(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const coord::BoxSides<FP, NDIM>& minimage_box,
    const envir::Environment<FP>& environment,
    mathtools::SquareAdjacencyMatrix& adjmat,
    FP cutoff_distance
)
{
    using Point = coord::Cartesian<FP, NDIM>;

    const auto n_particles = environment.n_particles();
    const auto n_timeslices = environment.n_timeslices();

    // TODO: put this somewhere else where I can use it repeatedly
    //       - probably in the worldlines subdirectory?
    const auto create_centroid = [&worldlines, &n_timeslices](std::size_t i_part) -> Point
    {
        auto centroid = Point::origin();
        for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
            centroid += worldlines[i_tslice][i_part];
        }
        centroid /= static_cast<FP>(n_timeslices);

        return centroid;
    };

    auto centroids = std::vector<Point> {};
    centroids.reserve(n_particles);
    for (std::size_t i_part {0}; i_part < n_particles; ++i_part) {
        centroids.push_back(create_centroid(i_part));
    }

    adjmat.clear_all();

    const auto cutoff_distance_sq = cutoff_distance * cutoff_distance;

    for (std::size_t ip0 {0}; ip0 < n_particles - 1; ++ip0) {
        for (std::size_t ip1 {ip0 + 1}; ip1 < n_particles; ++ip1) {
            const auto dist_sq = coord::distance_squared_periodic(centroids[ip0], centroids[ip1], minimage_box);
            if (dist_sq <= cutoff_distance_sq) {
                adjmat.add_neighbour_both(ip0, ip1);
            }
        }
    }
}

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires PairPointPotential<PointPotential, FP, NDIM>
class NearestNeighbourPairInteractionHandler
{
    using Worldline = worldline::Worldline<FP, NDIM>;

public:
    explicit NearestNeighbourPairInteractionHandler(PointPotential pot, std::size_t n_particles)
        : pot_ {std::move(pot)}
        , centroid_adjmat_ {n_particles}
    {}

    constexpr auto operator()(std::size_t i_particle, const Worldline& worldline) const noexcept -> FP
    {
        auto pot_energy = FP {};

        const auto& points = worldline.points();
        for (auto i_neigh : centroid_adjmat_.neighbours(i_particle)) {
            pot_energy += pot_(points[i_particle], points[i_neigh]);
        }

        return pot_energy;
    }

    constexpr auto adjacency_matrix() noexcept -> mathtools::SquareAdjacencyMatrix&
    {
        // mutable reference to the underlying adjacency matrix so an external function can update it
        return centroid_adjmat_;
    }

private:
    PointPotential pot_;
    mathtools::SquareAdjacencyMatrix centroid_adjmat_;
};

}  // namespace interact
