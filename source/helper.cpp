/*
    This source file is meant to be included into `main.cpp` as part of a unity build.
*/

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>

#include <argparser.hpp>
#include <geometries/bravais.hpp>
#include <geometries/lattice.hpp>
#include <geometries/lattice_type.hpp>
#include <geometries/unit_cell_translations.hpp>
#include <interactions/three_body/published/three_body_ibrahim2022.hpp>
#include <interactions/three_body/three_body_pointwise_wrapper.hpp>
#include <interactions/two_body/published/two_body_schmidt2015.hpp>
#include <interactions/two_body/two_body_pointwise_wrapper.hpp>
#include <mathtools/io/histogram.hpp>
#include <pimc/adjusters/adjusters.hpp>
#include <rng/generator.hpp>
#include <rng/prng_state.hpp>
#include <simulation/continue.hpp>
#include <worldline/worldline.hpp>
#include <worldline/writers/worldline_writer.hpp>

constexpr auto build_hcp_lattice_structure(
    auto density,
    auto n_unit_cells
)  //, std::size_t xsize, std::size_t ysize, std::size_t zsize)
{
    /* create the lattice positions and the periodic box */
    const auto lattice_type = geom::LatticeType::HCP;
    const auto lattice_constant = geom::density_to_lattice_constant(density, lattice_type);
    const auto hcp_unit_cell = geom::conventional_hcp_unit_cell(lattice_constant);
    const auto hcp_unit_cell_box = geom::unit_cell_box_sides(hcp_unit_cell);

    const auto [size0, size1, size2] = n_unit_cells;
    const auto lattice_box_translations = geom::UnitCellTranslations<3> {size0, size1, size2};

    const auto minimage_box = geom::lattice_box(hcp_unit_cell_box, lattice_box_translations);

    const auto lattice_site_positions = geom::lattice_particle_positions(hcp_unit_cell, lattice_box_translations);
    const auto n_particles = lattice_site_positions.size();

    return std::tuple(n_particles, minimage_box, lattice_site_positions);
}

template <std::floating_point FP>
auto fsh_potential(auto minimage_box, auto two_body_filepath)
{
    auto distance_pot = interact::two_body_schmidt2015<FP>(two_body_filepath);

    return interact::PeriodicTwoBodySquaredPointPotential {std::move(distance_pot), minimage_box};
}

auto threebodyparah2_potential(auto minimage_box, auto three_body_filepath)
{
    auto distance_pot = interact::three_body_ibrahim2022<float>(three_body_filepath);

    return interact::PeriodicThreeBodyPointPotential {std::move(distance_pot), minimage_box};
}

template <std::floating_point FP>
auto create_com_move_adjuster(FP lower_range_limit, FP upper_range_limit) noexcept
    -> pimc::SingleValueMoveAdjuster<FP>
{
    const auto com_accept_range = pimc::AcceptPercentageRange<FP> {lower_range_limit, upper_range_limit};
    const auto com_adjust_step = FP {0.005};
    const auto com_direction = pimc::DirectionIfAcceptTooLow::NEGATIVE;
    const auto com_limits = pimc::MoveLimits<FP> {FP {0.0}, std::nullopt};
    return pimc::SingleValueMoveAdjuster<FP> {com_accept_range, com_adjust_step, com_direction, com_limits};
}

template <std::floating_point FP>
auto create_bisect_move_adjuster(FP lower_range_limit, FP upper_range_limit, FP bisect_adjust_step) noexcept
    -> pimc::BisectionLevelMoveAdjuster<FP>
{
    const auto bisect_accept_range = pimc::AcceptPercentageRange<FP> {lower_range_limit, upper_range_limit};
    return pimc::BisectionLevelMoveAdjuster<FP> {bisect_accept_range, bisect_adjust_step};
}

template <std::floating_point FP, std::size_t NDIM>
auto create_histogram(
    const std::filesystem::path& histogram_filepath,
    const sim::ContinueFileManager& manager,
    const coord::BoxSides<FP, NDIM>& minimage_box
)
{
    if (manager.file_exists() && manager.get_info().is_equilibration_complete) {
        return mathtools::io::read_histogram<FP>(histogram_filepath);
    }
    else {
        return mathtools::Histogram<FP> {FP{0.0}, coord::box_cutoff_distance(minimage_box), 1024};
    }
}

template <std::floating_point FP>
auto read_simulation_first_block_index(
    const sim::ContinueFileManager& continue_file_manager,
    const argparse::ArgParser<FP>& parser
) -> std::size_t
{
    if (continue_file_manager.file_exists()) {
        // the continue file contains the most recently completed block, so we want the simulation
        // to start at the next block, hence the offset of 1 below
        if (continue_file_manager.get_info().is_at_least_one_worldline_index_saved) {
            return 1 + continue_file_manager.get_info().most_recent_saved_worldline_index;
        } else {
            return parser.first_block_index;
        }
    }
    else {
        return parser.first_block_index;
    }
}

auto create_prngw(
    const std::filesystem::path& prng_state_filepath,
    const std::variant<rng::RandomSeedFlag, std::uint64_t>& initial_seed_state
)
{
    if (std::filesystem::exists(prng_state_filepath) && std::filesystem::is_regular_file(prng_state_filepath)) {
        auto prngw = rng::RandomNumberGeneratorWrapper<std::mt19937>::from_uint64(0);
        rng::load_prng_state(prngw.prng(), prng_state_filepath);
        return prngw;
    }
    else if (std::holds_alternative<rng::RandomSeedFlag>(initial_seed_state)) {
        const auto flag = std::get<rng::RandomSeedFlag>(initial_seed_state);

        if (flag == rng::RandomSeedFlag::RANDOM) {
            return rng::RandomNumberGeneratorWrapper<std::mt19937>::from_random_uint64();
        }
        else if (flag == rng::RandomSeedFlag::TIME_SINCE_EPOCH) {
            return rng::RandomNumberGeneratorWrapper<std::mt19937>::from_time_since_epoch();
        }
        else {
            throw std::logic_error("unreachable path; PRNG flags exhausted");
        }
    }
    else {
        const auto value = std::get<std::uint64_t>(initial_seed_state);
        return rng::RandomNumberGeneratorWrapper<std::mt19937>::from_uint64(value);
    }
}

template <std::floating_point FP, std::size_t NDIM>
auto read_simulation_worldlines(
    const sim::ContinueFileManager& continue_file_manager,
    const worldline::WorldlineWriter<FP, NDIM>& worldline_writer,
    std::size_t n_timeslices,
    const std::vector<coord::Cartesian<FP, NDIM>>& lattice_site_positions
) -> worldline::Worldlines<FP, NDIM>
{
    if (continue_file_manager.is_continued()) {
        const auto info = continue_file_manager.get_info();

        if (info.is_at_least_one_worldline_index_saved) {
            const auto worldline_filepath = worldline_writer.output_filepath(info.most_recent_saved_worldline_index);
            return worldline::read_worldlines<FP, NDIM>(worldline_filepath);
        } else {
            return worldline::worldlines_from_positions<FP, NDIM>(lattice_site_positions, n_timeslices);
        }
    }
    else {
        return worldline::worldlines_from_positions<FP, NDIM>(lattice_site_positions, n_timeslices);
    }
}
