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
#include <interactions/handlers/interaction_handler_concepts.hpp>
#include <interactions/two_body/two_body_pointwise.hpp>
#include <pimc/trackers/move_success_tracker.hpp>
#include <rng/distributions.hpp>
#include <rng/generator.hpp>
#include <worldline/worldline.hpp>

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

    constexpr explicit CentreOfMassMovePerformer(std::size_t n_timeslices, FP step_size)
        : step_size_ {step_size}
        , position_cache_(n_timeslices, Point {})  // NOTE: initializing a vector: don't use list init here
    {
        check_step_size_(step_size);
    }

    constexpr void update_step_size(FP new_step_size)
    {
        check_step_size_(new_step_size);
        step_size_ = new_step_size;
    }

    constexpr auto step_size() const noexcept -> FP
    {
        return step_size_;
    }

    // TODO: wrap part of this in a try-catch block, to restore the original particle positions in case
    //       an exception is thrown
    constexpr void operator()(
        std::size_t i_particle,
        Worldlines& worldlines,
        rng::PRNGWrapper auto& prngw,
        interact::InteractionHandler auto& interact_handler,
        const envir::Environment<FP>& environment,
        MoveSuccessTracker* move_tracker = nullptr
    ) noexcept
    {
        const auto step = generate_step_(prngw);

        // calculate energy for the current configuration
        auto pot_energy_before = FP {};
        for (const auto& wline : worldlines) {
            pot_energy_before += interact_handler(i_particle, wline);
        }

        // save the current positions, and set the new ones
        for (std::size_t i_tslice {0}; i_tslice < worldlines.size(); ++i_tslice) {
            position_cache_[i_tslice] = worldlines[i_tslice][i_particle];
            worldlines[i_tslice][i_particle] += step;
        }

        // calculate energy for the new configuration
        auto pot_energy_after = FP {};
        for (const auto& wline : worldlines) {
            pot_energy_after += interact_handler(i_particle, wline);
        }

        const auto pot_energy_diff = pot_energy_after - pot_energy_before;
        if (pot_energy_diff >= FP {0.0}) {
            // if the energy does not decrease, we need to look further
            const auto boltz_factor = std::exp(-pot_energy_diff * environment.thermodynamic_tau());
            const auto rand01 = uniform_dist_.uniform_01(prngw);

            if (boltz_factor < rand01) {
                // the proposed move is rejected, restore the positions
                for (std::size_t i_tslice {0}; i_tslice < worldlines.size(); ++i_tslice) {
                    worldlines[i_tslice][i_particle] = position_cache_[i_tslice];
                }

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
    FP step_size_ {};
    std::vector<Point> position_cache_ {};
    rng::UniformFloatingPointDistribution<FP> uniform_dist_ {};

    void check_step_size_(FP step_size)
    {
        if (step_size < FP {0.0}) {
            auto err_msg = std::stringstream {};
            err_msg << "The step size entered to the CentreOfMassMovePerformer must be non-negative.\n";
            err_msg << "Found: " << std::setprecision(6) << step_size << '\n';
            throw std::runtime_error(err_msg.str());
        }
    }

    constexpr auto generate_step_(rng::PRNGWrapper auto& prngw) noexcept -> Point
    {
        auto step = Point {};
        for (std::size_t i {0}; i < NDIM; ++i) {
            step[i] = uniform_dist_.uniform_ab(FP {-1.0}, FP {1.0}, prngw) * step_size_;
        }

        return step;
    }
};

}  // namespace pimc
