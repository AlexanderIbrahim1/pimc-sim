#pragma once

#include <cmath>
#include <concepts>
#include <functional>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <interactions/handlers/periodic_full_pair_interaction_handler.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
#include <rng/distributions.hpp>
#include <rng/generator.hpp>
#include <worldline/worldline.hpp>

namespace pimc
{

template <std::floating_point FP, std::size_t NDIM>
class SingleBeadPositionMovePerformer
{
public:
    using Point = coord::Cartesian<FP, NDIM>;
    using Worldline = worldline::Worldline<FP, NDIM>;
    using Worldlines = std::vector<Worldline>;

    SingleBeadPositionMovePerformer() = delete;

    constexpr explicit SingleBeadPositionMovePerformer(std::size_t n_timeslices)
        : position_cache_(n_timeslices, Point {})  // NOTE: initializing a vector: don't use list init here
    {}

    // TODO: wrap part of this in a try-catch block, to restore the original particle positions in case
    //       an exception is thrown
    constexpr void operator()(
        std::size_t i_particle,
        std::size_t i_timeslice,
        Worldlines& worldlines,
        rng::PRNGWrapper auto& prngw,
        const interact::InteractionHandler auto& interact_handler,
        const envir::Environment<FP>& environment
    ) noexcept
    {
        const auto step = generate_step_(prngw);

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
            const auto rand01 = uniform_dist_.uniform_01(prngw);

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
    rng::UniformFloatingPointDistribution<FP> uniform_dist_ {};
    rng::NormalDistribution<FP> normal_dist_ {};

    constexpr auto generate_step_(FP step_stddev, rng::PRNGWrapper auto& prngw) noexcept -> Point
    {
        auto step = Point {};
        for (std::size_t i {0}; i < NDIM; ++i) {
            step[i] = normal_dist_.normal(FP {0.0}, step_stddev, prngw);
        }

        return step;
    }
};

}  // namespace pimc
