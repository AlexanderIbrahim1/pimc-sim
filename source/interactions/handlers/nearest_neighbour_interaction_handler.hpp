#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <utility>
#include <vector>

#include <coordinates/attard.hpp>
#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <interactions/four_body/potential_concepts.hpp>
#include <interactions/three_body/potential_concepts.hpp>
#include <interactions/two_body/potential_concepts.hpp>
#include <mathtools/grid/grid2d.hpp>
#include <mathtools/grid/square_adjacency_matrix.hpp>
#include <worldline/worldline.hpp>

namespace interact
{

template <std::floating_point FP, std::size_t NDIM>
auto create_centroid_pair_distance_squared_grid(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const coord::BoxSides<FP, NDIM>& minimage_box,
    const envir::Environment<FP>& environment
) -> mathtools::Grid2D<FP>
{
    const auto n_particles = environment.n_particles();
    auto grid = mathtools::Grid2D<FP> {n_particles, n_particles};

    const auto centroids = worldline::calculate_all_centroids(worldlines);
    for (std::size_t ip0 {0}; ip0 < centroids.size() - 1; ++ip0) {
        for (std::size_t ip1 {ip0 + 1}; ip1 < centroids.size(); ++ip1) {
            const auto dist_sq = coord::distance_squared_periodic(centroids[ip0], centroids[ip1], minimage_box);
            grid.set(ip0, ip1, dist_sq);
            grid.set(ip1, ip0, dist_sq);
        }
    }

    return grid;
}

template <std::floating_point FP>
void update_centroid_adjacency_matrix_from_grid(
    const mathtools::Grid2D<FP>& distance_squared_grid,
    mathtools::SquareAdjacencyMatrix& adjmat,
    FP cutoff_distance
)
{
    const auto n_particles = distance_squared_grid.n_rows();
    const auto cutoff_distance_sq = cutoff_distance * cutoff_distance;

    for (std::size_t ip0 {0}; ip0 < n_particles - 1; ++ip0) {
        for (std::size_t ip1 {ip0 + 1}; ip1 < n_particles; ++ip1) {
            if (distance_squared_grid.get(ip0, ip1) <= cutoff_distance_sq) {
                adjmat.add_neighbour_both(ip0, ip1);
            }
        }
    }
}

template <std::floating_point FP, std::size_t NDIM>
void update_centroid_adjacency_matrix(
    const std::vector<worldline::Worldline<FP, NDIM>>& worldlines,
    const coord::BoxSides<FP, NDIM>& minimage_box,
    const envir::Environment<FP>& environment,
    mathtools::SquareAdjacencyMatrix& adjmat,
    FP cutoff_distance
)
{
    const auto distance_sq_grid = create_centroid_pair_distance_squared_grid(worldlines, minimage_box, environment);
    update_centroid_adjacency_matrix_from_grid(distance_sq_grid, adjmat, cutoff_distance);
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

    constexpr auto point_potential() const -> const PointPotential&
    {
        return pot_;
    }

private:
    PointPotential pot_;
    mathtools::SquareAdjacencyMatrix centroid_adjmat_;
};

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires TripletPointPotential<PointPotential, FP, NDIM>
class NearestNeighbourTripletInteractionHandler
{
    using Worldline = worldline::Worldline<FP, NDIM>;

public:
    explicit NearestNeighbourTripletInteractionHandler(PointPotential pot, std::size_t n_particles)
        : pot_ {std::move(pot)}
        , centroid_adjmat_ {n_particles}
    {}

    constexpr auto operator()(std::size_t i_particle, const Worldline& worldline) const noexcept -> FP
    {
        // NOTE
        // this member function doesn't actually take periodicity into account; so the Attard
        //   nearest-neighbour correction won't be called;
        // instead, it assumes that the centroid adjacency matrix is tight enough that the nearest
        //   neighbours being considered in the interaction won't break the Attard convention;
        // for the simulations that I currently run, the box is large enough that this is always true
        auto pot_energy = FP {};

        const auto& points = worldline.points();
        const auto neighbours = centroid_adjmat_.neighbours(i_particle);

        for (std::size_t idx_neigh0 {0}; idx_neigh0 < neighbours.size() - 1; ++idx_neigh0) {
            for (std::size_t idx_neigh1 {idx_neigh0 + 1}; idx_neigh1 < neighbours.size(); ++idx_neigh1) {
                const auto i_neigh0 = neighbours[idx_neigh0];
                const auto i_neigh1 = neighbours[idx_neigh1];
                pot_energy += pot_(points[i_particle], points[i_neigh0], points[i_neigh1]);
            }
        }

        return pot_energy;
    }

    constexpr auto adjacency_matrix() noexcept -> mathtools::SquareAdjacencyMatrix&
    {
        // mutable reference to the underlying adjacency matrix so an external function can update it
        return centroid_adjmat_;
    }

    constexpr auto point_potential() const -> const PointPotential&
    {
        return pot_;
    }

private:
    PointPotential pot_;
    mathtools::SquareAdjacencyMatrix centroid_adjmat_;
};

template <typename Potential, std::floating_point FP, std::size_t NDIM>
requires BufferedQuadrupletPointPotential<Potential, FP, NDIM>
class NearestNeighbourQuadrupletInteractionHandler
{
    using Worldline = worldline::Worldline<FP, NDIM>;

public:
    explicit NearestNeighbourQuadrupletInteractionHandler(Potential pot, std::size_t n_particles)
        : pot_ {std::move(pot)}
        , centroid_adjmat_ {n_particles}
    {}

    constexpr auto operator()(std::size_t i_particle, const Worldline& worldline) const noexcept -> FP
    {
        // NOTE
        // this member function doesn't actually take periodicity into account; so the Attard
        //   nearest-neighbour correction won't be called;
        // instead, it assumes that the centroid adjacency matrix is tight enough that the nearest
        //   neighbours being considered in the interaction won't break the Attard convention;
        // for the simulations that I currently run, the box is large enough that this is always true
        const auto& points = worldline.points();
        const auto neighbours = centroid_adjmat_.neighbours(i_particle);

        for (std::size_t idx_neigh0 {0}; idx_neigh0 < neighbours.size() - 2; ++idx_neigh0) {
            for (std::size_t idx_neigh1 {idx_neigh0 + 1}; idx_neigh1 < neighbours.size() - 1; ++idx_neigh1) {
                for (std::size_t idx_neigh2 {idx_neigh1 + 1}; idx_neigh2 < neighbours.size(); ++idx_neigh2) {
                    const auto i_neigh0 = neighbours[idx_neigh0];
                    const auto i_neigh1 = neighbours[idx_neigh1];
                    const auto i_neigh2 = neighbours[idx_neigh2];
                    pot_.add_sample(points[i_particle], points[i_neigh0], points[i_neigh1], points[i_neigh2]);
                }
            }
        }

        const auto pot_energy = pot_.extract_energy();

        return pot_energy;
    }

    constexpr auto adjacency_matrix() noexcept -> mathtools::SquareAdjacencyMatrix&
    {
        // mutable reference to the underlying adjacency matrix so an external function can update it
        return centroid_adjmat_;
    }

    constexpr auto point_potential() const -> const PointPotential&
    {
        return pot_;
    }

private:
    Potential pot_;
    mathtools::SquareAdjacencyMatrix centroid_adjmat_;
};

}  // namespace interact
