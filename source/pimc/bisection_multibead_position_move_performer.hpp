#pragma once

#include <concepts>
#include <numeric>
#include <stdexcept>
#include <vector>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <interactions/handlers/interaction_handler_concepts.hpp>
#include <pimc/bisection_level_manager.hpp>
#include <pimc/trackers/move_success_tracker.hpp>
#include <rng/distributions.hpp>
#include <rng/generator.hpp>
#include <worldline/worldline.hpp>

namespace pimc
{

template <std::floating_point FP, std::size_t NDIM>
class BisectionMultibeadPositionMovePerformer
{
public:
    using Point = coord::Cartesian<FP, NDIM>;
    using Worldline = worldline::Worldline<FP, NDIM>;

    BisectionMultibeadPositionMovePerformer() = delete;

    constexpr explicit BisectionMultibeadPositionMovePerformer(FP lower_level_frac, std::size_t lower_level)
        : lower_level_frac_ {lower_level_frac}
        , lower_level_ {lower_level}
    {
        check_lower_level_frac_(lower_level_frac_);
        check_lower_level_(lower_level_);
    }

    constexpr void operator()(
        std::size_t i_particle,
        std::size_t i_timeslice,
        std::vector<Worldline>& worldlines,
        rng::PRNGWrapper auto& prngw,
        const interact::InteractionHandler auto& interact_handler,
        const envir::Environment<FP>& environment,
        MoveSuccessTracker* move_tracker = nullptr
    )
    {
        const auto level = choose_bisection_level_(prngw);
        const auto bisection_level_manager = BisectionLevelManager {level, i_timeslice, environment.n_timeslices()};
        const auto original_position_cache = create_original_position_cache(i_timeslice, i_particle, level, worldlines);

        auto flag_accept_move = bool {true};

        for (std::size_t sublevel {0}; sublevel < level; ++sublevel) {
            // calculate energy for current configuration
            auto pot_energy_before = FP {0.0};
            for (const auto bisect_trip : bisection_level_manager.triplets(sublevel)) {
                pot_energy_before += interact_handler(i_particle, worldlines[bisect_trip.mid]);
            }

            // set proposed positions
            const auto step_stddev = step_stddev_(environment, level, sublevel);
            for (const auto bisect_trip : bisection_level_manager.triplets(sublevel)) {
                const auto left = worldlines[bisect_trip.left][i_particle];
                const auto right = worldlines[bisect_trip.right][i_particle];
                const auto step = generate_step_(prngw, step_stddev);
                const auto proposed = 0.5 * (left + right) + step;

                worldlines[bisect_trip.mid][i_particle] = proposed;
            }

            // calculate energy for proposed configuration
            auto pot_energy_after = FP {0.0};
            for (const auto bisect_trip : bisection_level_manager.triplets(sublevel)) {
                pot_energy_after += interact_handler(i_particle, worldlines[bisect_trip.mid]);
            }

            const auto pot_energy_diff = pot_energy_after - pot_energy_before;
            if (pot_energy_diff >= FP {0.0}) {
                // if the energy does not decrease, we need to look further
                const auto boltz_factor = std::exp(-pot_energy_diff * environment.thermodynamic_tau());
                const auto rand01 = uniform_dist_.uniform_01(prngw);

                if (boltz_factor < rand01) {
                    // the proposed move is rejected, restore the positions
                    flag_accept_move = false;
                    break;
                }
            }
        }

        if (!flag_accept_move) {
            // NOTE: the first and last positions were never modified; they were only used to calculate
            // the proposed steps in-between
            for (std::size_t i {1}; i < original_position_cache.size() - 1; ++i) {
                const auto i_tslice = (i + i_timeslice) % environment.n_timeslices();
                worldlines[i_tslice][i_particle] = original_position_cache[i];
            }

            if (move_tracker) {
                move_tracker->add_reject();
            }
        } else {
            if (move_tracker) {
                move_tracker->add_accept();
            }
        }
    }

private:
    FP lower_level_frac_;
    std::size_t lower_level_;
    rng::UniformFloatingPointDistribution<FP> uniform_dist_ {};
    rng::NormalDistribution<FP> normal_dist_ {};

    constexpr auto create_original_position_cache(
        std::size_t i_timeslice,
        std::size_t i_particle,
        std::size_t level,
        const std::vector<Worldline>& worldlines
    ) const noexcept -> std::vector<Point>
    {
        const auto n_timeslices = worldlines.size();  // I think somewhere else we guarantee that n_timeslices > 0?
        const auto size = level_segment_size(level) + 1;

        auto cache = std::vector<Point> {};
        cache.reserve(size);

        for (std::size_t i {0}; i < size; ++i) {
            const auto it = (i_timeslice + i) % n_timeslices;
            cache.push_back(worldlines[it][i_particle]);
        }

        return cache;
    }

    constexpr void check_lower_level_frac_(FP lower_level_frac) const
    {
        if (lower_level_frac < FP {0.0} || lower_level_frac >= FP {1.0}) {
            throw std::runtime_error(
                "The lower level fraction for the bisection multibead position move "
                "must be between 0.0 and 1.0\n"
            );
        }
    }

    constexpr void check_lower_level_(std::size_t lower_level) const
    {
        if (lower_level < 1) {
            throw std::runtime_error("The lower level for the bisection multibead position move must be 1 or greater.\n"
            );
        }
    }

    constexpr auto choose_bisection_level_(rng::PRNGWrapper auto& prngw) noexcept -> std::size_t
    {
        const auto rand01 = uniform_dist_.uniform_01(prngw);
        if (rand01 < lower_level_frac_) {
            return lower_level_;
        }
        else {
            return lower_level_ + 1;
        }
    }

    constexpr auto step_stddev_(const envir::Environment<FP>& environment, std::size_t level, std::size_t sublevel)
        const noexcept -> FP
    {
        const auto level_factor = static_cast<FP>(pow_int(2, level - sublevel - 1));
        const auto lambda = environment.thermodynamic_lambda();
        const auto tau = environment.thermodynamic_tau();

        return std::sqrt(level_factor * lambda * tau);
    }

    constexpr auto generate_step_(rng::PRNGWrapper auto& prngw, FP step_stddev) noexcept -> Point
    {
        auto step = Point {};
        for (std::size_t i {0}; i < NDIM; ++i) {
            step[i] = normal_dist_.normal(FP {0.0}, step_stddev, prngw);
        }

        return step;
    }
};

}  // namespace pimc
