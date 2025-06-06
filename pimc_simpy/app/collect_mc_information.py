"""
This script contains code for collecting information about the optimal Monte Carlo
step size information from the simulations run so far.
"""

import sys
from pathlib import Path

from pimc_simpy.data import PropertyData
from pimc_simpy.data import last_n_epochs
from pimc_simpy.data import between_epochs
from pimc_simpy.data import get_property_statistics

from pimc_simpy.quick_analysis import ProjectDataReader
from pimc_simpy.quick_analysis import write_converged_bisection_multibead_position_move_info_last
from pimc_simpy.quick_analysis import write_converged_centre_of_mass_step_size_last
from pimc_simpy.quick_analysis import read_converged_bisection_multibead_position_move_info
from pimc_simpy.quick_analysis import read_converged_centre_of_mass_step_size

from pimc_simpy.manage import parse_project_info
from pimc_simpy.manage import ProjectInfo
from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager

from pimc_simpy.plotting import plot_property


def plot_monte_carlo_acceptance_rates(reader: ProjectDataReader, sim_id: int) -> None:
    bisect_accepts, bisect_rejects = reader.read_project_bisection_multibead_position_move_acceptance(sim_id)
    single_accepts, single_rejects = reader.read_project_single_bead_position_move_acceptance(sim_id)
    centre_accepts, centre_rejects = reader.read_project_centre_of_mass_position_move_acceptance(sim_id)

    bisect_accept_rates = bisect_accepts / (bisect_accepts + bisect_rejects)
    single_accept_rates = single_accepts / (single_accepts + single_rejects)
    centre_accept_rates = centre_accepts / (centre_accepts + centre_rejects)

    plot_property([bisect_accept_rates, single_accept_rates, centre_accept_rates], labels=["bisect", "single", "centre"])


def plot_monte_carlo_steps(reader: ProjectDataReader, sim_id: int) -> None:
    upper_fractions, lower_levels = reader.read_project_bisection_multibead_position_move_info(sim_id)
    com_step_sizes = reader.read_project_centre_of_mass_step_size(sim_id)

    plot_property([upper_fractions, lower_levels, com_step_sizes], labels=["upper_fractions", "lower_levels", "com_step_sizes"])


def converged_monte_carlo_steps(reader: ProjectDataReader, sim_id: int, n_last: int) -> None:
    upper_fractions, lower_levels = reader.read_project_bisection_multibead_position_move_info(sim_id)
    com_step_sizes = reader.read_project_centre_of_mass_step_size(sim_id)

    def print_mean_and_sem(data: PropertyData) -> None:
        data_ = last_n_epochs(n_last, data)
        stats = get_property_statistics(data_)

        print(f"(mean, sem) = ({stats.mean}, {stats.stderrmean})")

    print_mean_and_sem(upper_fractions)
    print_mean_and_sem(lower_levels)
    print_mean_and_sem(com_step_sizes)


def get_reader(project_info_toml_filepath: Path) -> ProjectDataReader:
    info = parse_project_info(project_info_toml_filepath)
    formatter = BasicProjectDirectoryFormatter()
    manager = ProjectDirectoryStructureManager(info, formatter)

    return ProjectDataReader(manager)


# -----------------------------------------------------------------------------

# PROJECT_INFO_TOML_FILEPATH = Path("..", "project_info_toml_files", "local_p960_coarse_pert2b3b_mcmc_param_search.toml")
PERT2B3B4B_PARAM_SEARCH_DIRPATH = Path("..", "playground", "cedar_files", "mcmc_param_search_pert2b3b4b")
PROJECT_INFO_TOML_FILEPATH = PERT2B3B4B_PARAM_SEARCH_DIRPATH / "local_mcmc_param_search_p64.toml"


def write_converged_move_infos() -> None:
    # bisection_output_filepath = Path("..", "playground", "converged_bisection_move_info_pert2b3b_p960.dat")
    # com_output_filepath = Path("..", "playground", "converged_centre_of_mass_step_size_pert2b3b_p960.dat")
    bisection_output_filepath = PERT2B3B4B_PARAM_SEARCH_DIRPATH / "recent_bisection_move_info_pert2b3b4b.dat"
    com_output_filepath = PERT2B3B4B_PARAM_SEARCH_DIRPATH / "recent_centre_of_mass_step_size_pert2b3b4b"

    reader = get_reader(PROJECT_INFO_TOML_FILEPATH)

    simulation_indices = list(range(31))
    n_last: int = 3

    write_converged_bisection_multibead_position_move_info_last(reader, bisection_output_filepath, simulation_indices, n_last)
    write_converged_centre_of_mass_step_size_last(reader, com_output_filepath, simulation_indices, n_last)


def plot_move_infos() -> None:
    sim_id = int(sys.argv[1])
    reader = get_reader(PROJECT_INFO_TOML_FILEPATH)

    plot_monte_carlo_acceptance_rates(reader, sim_id)
    # plot_monte_carlo_steps(reader, sim_id)
    # converged_monte_carlo_steps(reader, sim_id, 50)


if __name__ == "__main__":
    plot_move_infos()
    # write_converged_move_infos()
