#pragma once

#include <concepts>
#include <vector>

#include <coordinates/cartesian.hpp>
#include <worldline/worldline.hpp>

// PLAN:
// - pick a particle (need index, worldlines)
// - calculate the current potential energy for that particle only
// - store all the positions for that particle (in case the move fails)
// - write the new positions into the worldlines
// - calculate the new potential energy for that particle only
// - maybe accept the new positions based on the energy difference

// NEEDS:
// - number of particles for the cache

// DESIGN:
// - the environment info (temperature, etc.), PRNG, and worldlines should be passed to the
//   function call, and not stored in the mover as a state
//   - the performance impact would be negligible, and this makes the function more flexible
// - the number of particles should be stored as a state
//   - because we don't want to regenerate the cache every single time
//   - unless we provide the cache externally?
//     - but that would be very annoying for the user
// - the action difference calculator, and the move producer, should be passed in separately

// TODO:
// - add a move acceptance ratio logger


namespace pimc {

template <std::floating_point FP, std::size_t NDIM>
class CentreOfMassMovePerformer {
public:
    using Point = coord::Cartesian<FP, NDIM>;

    CentreOfMassMovePerformer() = delete;

    constexpr explicit CentreOfMassMovePerformer(std::size_t n_particles)
        : particle_position_cache_(n_particles, Point{})
    {}

    // index, all worldlines, PRNG, interactions, callable to create the new positions
private:
    std::vector<Point> particle_position_cache_ {};
};

}  // namespace pimc
