#pragma once

#include <cmath>
#include <concepts>
#include <functional>
#include <sstream>
#include <utility>
#include <vector>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <interactions/handlers/interaction_handler_concepts.hpp>
#include <pimc/trackers/move_success_tracker.hpp>
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

    SingleBeadPositionMovePerformer() = delete;

    constexpr explicit SingleBeadPositionMovePerformer(std::size_t n_timeslices)
        : n_timeslices_ {n_timeslices}
    {}

    constexpr void operator()(
        std::size_t i_particle,
        std::size_t i_timeslice,
        worldline::Worldlines<FP, NDIM>& worldlines,
        rng::PRNGWrapper auto& prngw,
        interact::InteractionHandler<FP, NDIM> auto& interact_handler,
        const envir::Environment<FP>& environment,
        MoveSuccessTracker* move_tracker = nullptr
    ) noexcept
    {
        const auto proposed_bead_mean = proposed_bead_position_mean_(i_timeslice, i_particle, worldlines);
        const auto step = generate_step_(environment, prngw);
        const auto proposed_bead = proposed_bead_mean + step;

        // calculate energy for the current configuration
        const auto pot_energy_before = interact_handler(i_timeslice, i_particle, worldlines);

        // save the current position, and set the new one
        const auto current_bead = worldlines.get(i_timeslice, i_particle);
        worldlines.set(i_timeslice, i_particle, proposed_bead);

        // calculate energy for the proposed configuration
        const auto pot_energy_after = interact_handler(i_timeslice, i_particle, worldlines);

        const auto pot_energy_diff = pot_energy_after - pot_energy_before;
        if (pot_energy_diff >= FP {0.0}) {
            // if the energy does not decrease, we need to look further
            const auto boltz_factor = std::exp(-pot_energy_diff * environment.thermodynamic_tau());
            const auto rand01 = uniform_dist_.uniform_01(prngw);

            if (boltz_factor < rand01) {
                // the proposed move is rejected, restore the positions
                worldlines.set(i_timeslice, i_particle, current_bead);

                if (move_tracker) {
                    move_tracker->add_reject();
                }
            }
            else {
                if (move_tracker) {
                    move_tracker->add_accept();
                }
            }
        }
        else {
            if (move_tracker) {
                move_tracker->add_accept();
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
        std::size_t i_timeslice,
        std::size_t i_particle,
        const worldline::Worldlines<FP, NDIM>& worldlines
    ) const noexcept -> Point
    {
        const auto it_before = (i_timeslice + n_timeslices_ - 1) % n_timeslices_;
        const auto it_after = (i_timeslice + 1) % n_timeslices_;

        const auto& bead_before = worldlines.get(it_before, i_particle);
        const auto& bead_after = worldlines.get(it_after, i_particle);

        return FP {0.5} * (bead_before + bead_after);
    }
};

}  // namespace pimc
