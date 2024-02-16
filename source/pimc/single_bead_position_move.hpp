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
        : n_timeslices_ {n_timeslices}
    {}

    constexpr void operator()(
        std::size_t i_particle,
        std::size_t i_timeslice,
        Worldlines& worldlines,
        rng::PRNGWrapper auto& prngw,
        const interact::InteractionHandler auto& interact_handler,
        const envir::Environment<FP>& environment
    ) noexcept
    {
        const auto proposed_bead_mean = proposed_bead_position_mean_(i_particle, i_timeslice, worldlines);
        const auto step = generate_step_(environment, prngw);
        const auto proposed_bead = proposed_bead_mean + step;

        // calculate energy for the current configuration
        const auto pot_energy_before = interact_handler(i_particle, worldlines[i_timeslice]);

        // save the current position, and set the new one
        const auto current_bead = worldlines[i_timeslice][i_particle];
        worldlines[i_timeslice][i_particle] = proposed_bead;

        // calculate energy for the proposed configuration
        const auto pot_energy_after = interact_handler(i_particle, worldlines[i_timeslice]);

        const auto pot_energy_diff = pot_energy_after - pot_energy_before;
        if (pot_energy_diff >= FP {0.0}) {
            // if the energy does not decrease, we need to look further
            const auto boltz_factor = std::exp(-pot_energy_diff * environment.thermodynamic_tau());
            const auto rand01 = uniform_dist_.uniform_01(prngw);

            if (boltz_factor < rand01) {
                // the proposed move is rejected, restore the positions
                worldlines[i_timeslice][i_particle] = current_bead;
            }
        }
    }

private:
    std::size_t n_timeslices_ {};

    rng::UniformFloatingPointDistribution<FP> uniform_dist_ {};
    rng::NormalDistribution<FP> normal_dist_ {};

    constexpr auto generate_step_(const envir::Environment<FP>& environment, rng::PRNGWrapper auto& prngw) noexcept
        -> Point
    {
        const auto lambda = environment.thermodynamic_lambda();
        const auto tau = environment.thermodynamic_tau();
        const auto step_stddev = std::sqrt(lambda * tau);

        auto step = Point {};
        for (std::size_t i {0}; i < NDIM; ++i) {
            step[i] = normal_dist_.normal(FP {0.0}, step_stddev, prngw);
        }

        return step;
    }

    constexpr auto proposed_bead_position_mean_(
        std::size_t i_particle,
        std::size_t i_timeslice,
        const Worldlines& worldlines
    ) const noexcept -> Point
    {
        const auto it_before = (i_timeslice + n_timeslices_ - 1) % n_timeslices_;
        const auto it_after = (i_timeslice + 1) % n_timeslices_;

        const auto bead_before = worldlines[it_before][i_particle];
        const auto bead_after = worldlines[it_after][i_particle];

        return 0.5 * (bead_before + bead_after);
    }
};

}  // namespace pimc
