#pragma once

#include <cmath>
#include <concepts>
#include <functional>
#include <utility>
#include <vector>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
#include <interactions/handlers/periodic_full_pair_interaction_handler.hpp>
#include <rng/distributions.hpp>
#include <rng/generator.hpp>
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

namespace pimc
{

template <std::floating_point FP, std::size_t NDIM>
class CentreOfMassMovePerformer
{
public:
    using Point = coord::Cartesian<FP, NDIM>;
    using Worldline = worldline::Worldline<FP, NDIM>;
    using Worldlines = std::vector<Worldline>;

    CentreOfMassMovePerformer() = delete;

    constexpr explicit CentreOfMassMovePerformer(std::size_t n_timeslices)
        : position_cache_(n_timeslices, Point {})  // NOTE: initializing a vector: don't use list init here
    {}

    // TODO: wrap part of this in a try-catch block, to restore the original particle positions in case
    //       an exception is thrown
    constexpr void operator()(
        std::size_t i_particle,
        Worldlines& worldlines,
        rng::PRNGWrapper auto& prngw,
        const interact::InteractionHandler& interact_handler,
        const std::function<void(Point)>& step_generator,
        const envir::Environment<FP>& environment
    ) noexcept
    {
        const auto step = step_generator();

        // calculate energy for the current configuration
        auto pot_energy_before = FP {};
        for (const auto wline : worldlines) {
            pot_energy_before += interact_handler(i_particle, wline);
        }

        // save the current positions, and set the new ones
        for (std::size_t i_tslice {0}; i_tslice < worldlines->size(); ++i_tslice) {
            position_cache_[i_tslice] = worldlines[i_tslice][i_particle];
            worldlines[i_tslice][i_particle] += step;
        }

        // calculate energy for the new configuration
        auto pot_energy_after = FP {};
        for (const auto wline : worldlines) {
            pot_energy_after += interact_handler(i_particle, wline);
        }

        const auto pot_energy_diff = pot_energy_after - pot_energy_before;
        if (pot_energy_diff >= FP {0.0}) {
            // if the energy does not decrease, we need to look further
            const auto boltz_factor = std::exp(-pot_energy_diff * environment.thermodynamic_tau());
            const auto rand01 = uniform01_dist_.uniform_01(prngw);

            if (boltz_factor < rand01) {
                // restore the positions
                for (std::size_t i_tslice {0}; i_tslice < worldlines->size(); ++i_tslice) {
                    worldlines[i_tslice][i_particle] = position_cache_[i_tslice];
                }
            }
        }
    }

private:
    std::vector<Point> position_cache_ {};
    rng::UniformFloatingPointDistribution<FP> uniform01_dist_ {};
};

}  // namespace pimc
