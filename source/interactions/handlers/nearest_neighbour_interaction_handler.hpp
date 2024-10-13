#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <span>
#include <utility>
#include <vector>

#include <coordinates/attard/four_body.hpp>
#include <coordinates/box_sides.hpp>
#include <coordinates/measure.hpp>
#include <coordinates/measure_concepts.hpp>
#include <coordinates/operations.hpp>
#include <environment/environment.hpp>
#include <interactions/four_body/potential_concepts.hpp>
#include <interactions/three_body/potential_concepts.hpp>
#include <interactions/two_body/potential_concepts.hpp>
#include <mathtools/grid/grid2d.hpp>
#include <mathtools/grid/square_adjacency_matrix.hpp>
#include <worldline/worldline.hpp>

namespace interact
{

template <std::floating_point FP>
void update_centroid_adjacency_matrix_from_grid(
    const mathtools::Grid2D<FP>& distance_squared_grid,
    mathtools::SquareAdjacencyMatrix& adjmat,
    FP cutoff_distance
)
{
    const auto n_particles = distance_squared_grid.n_rows();
    const auto cutoff_distance_sq = cutoff_distance * cutoff_distance;

    adjmat.clear_all();

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
    const worldline::Worldlines<FP, NDIM>& worldlines,
    const coord::DistanceSquaredCalculator<FP, NDIM> auto& distance_squared_calculator,
    mathtools::SquareAdjacencyMatrix& adjmat,
    FP cutoff_distance
)
{
    const auto centroids = worldline::calculate_all_centroids(worldlines);
    const auto distance_sq_grid = coord::create_pair_measure_grid(centroids, distance_squared_calculator);

    update_centroid_adjacency_matrix_from_grid(distance_sq_grid, adjmat, cutoff_distance);
}

template <typename PointPotential, std::floating_point FP, std::size_t NDIM>
requires PairPointPotential<PointPotential, FP, NDIM>
class NearestNeighbourPairInteractionHandler
{
public:
    explicit NearestNeighbourPairInteractionHandler(PointPotential pot, std::size_t n_particles)
        : pot_ {std::move(pot)}
        , centroid_adjmat_ {n_particles}
    {}

    constexpr auto operator()(std::size_t i_timeslice, std::size_t i_particle, const worldline::Worldlines<FP, NDIM>& worldlines) noexcept -> FP
    {
        const auto timeslice = worldlines.timeslice(i_timeslice);
        const auto particle = timeslice[i_particle];

        auto pot_energy = FP {};

        for (auto i_neigh : centroid_adjmat_.neighbours(i_particle)) {
            pot_energy += pot_(particle, timeslice[i_neigh]);
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
public:
    explicit NearestNeighbourTripletInteractionHandler(PointPotential pot, std::size_t n_particles)
        : pot_ {std::move(pot)}
        , centroid_adjmat_ {n_particles}
    {}

    constexpr auto operator()(std::size_t i_timeslice, std::size_t i_particle, const worldline::Worldlines<FP, NDIM>& worldlines) noexcept -> FP
    {
        // NOTE
        // this member function doesn't actually take periodicity into account; so the Attard
        //   nearest-neighbour correction won't be called;
        // instead, it assumes that the centroid adjacency matrix is tight enough that the nearest
        //   neighbours being considered in the interaction won't break the Attard convention;
        // for the simulations that I currently run, the box is large enough that this is always true
        const auto neighbours = centroid_adjmat_.neighbours(i_particle);
        const auto timeslice = worldlines.timeslice(i_timeslice);
        const auto particle = timeslice[i_particle];

        auto pot_energy = FP {};

        for (std::size_t idx_neigh0 {0}; idx_neigh0 < neighbours.size() - 1; ++idx_neigh0) {
            for (std::size_t idx_neigh1 {idx_neigh0 + 1}; idx_neigh1 < neighbours.size(); ++idx_neigh1) {
                const auto i_neigh0 = neighbours[idx_neigh0];
                const auto i_neigh1 = neighbours[idx_neigh1];
                pot_energy += pot_(particle, timeslice[i_neigh0], timeslice[i_neigh1]);
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
requires BufferedQuadrupletPotential<Potential, FP>
class NearestNeighbourQuadrupletInteractionHandler
{
public:
    explicit NearestNeighbourQuadrupletInteractionHandler(Potential pot, std::size_t n_particles)
        : pot_ {std::move(pot)}
        , centroid_adjmat_ {n_particles}
    {}

    constexpr auto operator()(std::size_t i_timeslice, std::size_t i_particle, const worldline::Worldlines<FP, NDIM>& worldlines) noexcept -> FP
    {
        // NOTE
        // this member function doesn't actually take periodicity into account; so the Attard
        //   nearest-neighbour correction won't be called;
        // instead, it assumes that the centroid adjacency matrix is tight enough that the nearest
        //   neighbours being considered in the interaction won't break the Attard convention;
        // for the simulations that I currently run, the box is large enough that this is always true
        const auto neighbours = centroid_adjmat_.neighbours(i_particle);
        const auto timeslice = worldlines.timeslice(i_timeslice);

        const auto point0 = timeslice[i_particle];

        for (std::size_t idx_neigh1 {0}; idx_neigh1 < neighbours.size() - 2; ++idx_neigh1) {
            const auto i_neigh1 = neighbours[idx_neigh1];
            const auto point1 = timeslice[i_neigh1];
            const auto dist01 = coord::distance(point0, point1);

            for (std::size_t idx_neigh2 {idx_neigh1 + 1}; idx_neigh2 < neighbours.size() - 1; ++idx_neigh2) {
                const auto i_neigh2 = neighbours[idx_neigh2];
                const auto point2 = timeslice[i_neigh2];
                const auto dist02 = coord::distance(point0, point2);
                const auto dist12 = coord::distance(point1, point2);

                for (std::size_t idx_neigh3 {idx_neigh2 + 1}; idx_neigh3 < neighbours.size(); ++idx_neigh3) {
                    const auto i_neigh3 = neighbours[idx_neigh3];
                    const auto point3 = timeslice[i_neigh3];
                    const auto dist03 = coord::distance(point0, point3);
                    const auto dist13 = coord::distance(point1, point3);
                    const auto dist23 = coord::distance(point2, point3);

                    pot_.add_sample(coord::FourBodySideLengths<FP> {dist01, dist02, dist03, dist12, dist13, dist23});
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

    constexpr auto point_potential() -> Potential&
    {
        return pot_;
    }

private:
    Potential pot_;
    mathtools::SquareAdjacencyMatrix centroid_adjmat_;
};

}  // namespace interact
