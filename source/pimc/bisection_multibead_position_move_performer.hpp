#pragma once

#include <concepts>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <coordinates/box_sides.hpp>
#include <coordinates/cartesian.hpp>
#include <coordinates/measure.hpp>
#include <environment/environment.hpp>
#include <interactions/handlers/interaction_handler_concepts.hpp>
#include <pimc/bisection_level_manager.hpp>
#include <pimc/bisection_level_move_info.hpp>
#include <pimc/trackers/move_success_tracker.hpp>
#include <rng/distributions.hpp>
#include <rng/generator.hpp>
#include <worldline/worldline.hpp>

namespace pimc
{

namespace pimc_utils
{

// sometimes the values might be just below 0.0 or just above 1.0 due to floating-point errors;
// this error isn't crucial for the functionality of the move adjustment, as long as they aren't
// extremely away from the bounds; these tolerances cover that
template <std::floating_point FP>
constexpr auto UPPER_LEVEL_FRAC_MINIMUM_TOLERANCE = static_cast<FP>(-1.0e-3);

template <std::floating_point FP>
constexpr auto UPPER_LEVEL_FRAC_MAXIMUM_TOLERANCE = static_cast<FP>(1.0 + 1.0e-3);

}  // namespace pimc_utils

}  // namespace pimc

namespace pimc
{

template <std::floating_point FP, std::size_t NDIM>
class BisectionMultibeadPositionMovePerformer
{
public:
    using Point = coord::Cartesian<FP, NDIM>;

    BisectionMultibeadPositionMovePerformer() = delete;

    explicit BisectionMultibeadPositionMovePerformer(BisectionLevelMoveInfo<FP> move_info)
        : move_info_ {move_info}
    {
        check_upper_level_frac_(move_info_.upper_level_frac);
        check_lower_level_(move_info_.lower_level);
    }

    void update_bisection_level_move_info(BisectionLevelMoveInfo<FP> move_info)
    {
        check_upper_level_frac_(move_info_.upper_level_frac);
        check_lower_level_(move_info_.lower_level);
        move_info_ = move_info;
    }

    constexpr auto bisection_level_move_info() const noexcept -> BisectionLevelMoveInfo<FP>
    {
        return move_info_;
    }

    constexpr void operator()(
        std::size_t i_particle,
        std::size_t i_timeslice,
        worldline::Worldlines<FP, NDIM>& worldlines,
        rng::PRNGWrapper auto& prngw,
        interact::InteractionHandler<FP, NDIM> auto& interact_handler,
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
                pot_energy_before += interact_handler(bisect_trip.mid, i_particle, worldlines);
            }

            // set proposed positions
            const auto step_stddev = step_stddev_(environment, level, sublevel);
            for (const auto bisect_trip : bisection_level_manager.triplets(sublevel)) {
                const auto left = worldlines.get(bisect_trip.left, i_particle);
                const auto right = worldlines.get(bisect_trip.right, i_particle);
                const auto step = generate_step_(prngw, step_stddev);
                const auto proposed = FP {0.5} * (left + right) + step;

                worldlines.set(bisect_trip.mid, i_particle, proposed);
            }

            // calculate energy for proposed configuration
            auto pot_energy_after = FP {0.0};
            for (const auto bisect_trip : bisection_level_manager.triplets(sublevel)) {
                pot_energy_after += interact_handler(bisect_trip.mid, i_particle, worldlines);
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
                worldlines.set(i_tslice, i_particle, original_position_cache[i]);
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

private:
    BisectionLevelMoveInfo<FP> move_info_;
    rng::UniformFloatingPointDistribution<FP> uniform_dist_ {};
    rng::NormalDistribution<FP> normal_dist_ {};

    constexpr auto create_original_position_cache(
        std::size_t i_timeslice,
        std::size_t i_particle,
        std::size_t level,
        const worldline::Worldlines<FP, NDIM>& worldlines
    ) const noexcept -> std::vector<Point>
    {
        const auto n_timeslices =
            worldlines.n_timeslices();  // I think somewhere else we guarantee that n_timeslices > 0?
        const auto size = level_segment_size(level) + 1;

        auto cache = std::vector<Point> {};
        cache.reserve(size);

        for (std::size_t i {0}; i < size; ++i) {
            const auto it = (i_timeslice + i) % n_timeslices;
            cache.push_back(worldlines.get(it, i_particle));
        }

        return cache;
    }

    void check_upper_level_frac_(FP upper_level_frac) const
    {
        const auto minimum = pimc_utils::UPPER_LEVEL_FRAC_MINIMUM_TOLERANCE<FP>;
        const auto maximum = pimc_utils::UPPER_LEVEL_FRAC_MAXIMUM_TOLERANCE<FP>;
        if (upper_level_frac < minimum || upper_level_frac >= maximum) {
            auto err_msg = std::stringstream {};
            err_msg << "The upper level fraction for the bisection multibead position move\n";
            err_msg << "must be between 0.0 and 1.0\n";
            err_msg << "Found: " << std::fixed << std::setprecision(8) << upper_level_frac << '\n';
            throw std::runtime_error {err_msg.str()};
        }
    }

    void check_lower_level_(std::size_t lower_level) const
    {
        if (lower_level < 1) {
            throw std::runtime_error {"The lower level for the bisection multibead position move must at least 1.\n"};
        }
    }

    constexpr auto choose_bisection_level_(rng::PRNGWrapper auto& prngw) noexcept -> std::size_t
    {
        const auto rand01 = uniform_dist_.uniform_01(prngw);
        if (rand01 < move_info_.upper_level_frac) {
            return move_info_.lower_level + 1;
        }
        else {
            return move_info_.lower_level;
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
