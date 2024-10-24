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
    # upper_fractions, lower_levels = reader.read_project_bisection_multibead_position_move_info(sim_id)
    # com_step_sizes = reader.read_project_centre_of_mass_step_size(sim_id)

    bisect_accept_rates = bisect_accepts / (bisect_accepts + bisect_rejects)
    single_accept_rates = single_accepts / (single_accepts + single_rejects)
    centre_accept_rates = centre_accepts / (centre_accepts + centre_rejects)

    plot_property([bisect_accept_rates, single_accept_rates, centre_accept_rates], labels=["bisect", "single", "centre"])


def plot_monte_carlo_steps(reader: ProjectDataReader, sim_id: int) -> None:
    upper_fractions, lower_levels = reader.read_project_bisection_multibead_position_move_info(sim_id)
    com_step_sizes = reader.read_project_centre_of_mass_step_size(sim_id)

    plot_property([upper_fractions, lower_levels, com_step_sizes], labels=["upper_fractions", "lower_levels", "com_step_sizes"])


def converged_monte_carlo_steps(reader: ProjectDataReader, sim_id: int, between: tuple[int, int]) -> None:
    upper_fractions, lower_levels = reader.read_project_bisection_multibead_position_move_info(sim_id)
    com_step_sizes = reader.read_project_centre_of_mass_step_size(sim_id)

    def print_mean_and_sem(data: PropertyData) -> None:
        data = between_epochs(between[0], between[1], data)
        stats = get_property_statistics(data)

        print(f"(mean, sem) = ({stats.mean}, {stats.stderrmean})")

    print_mean_and_sem(upper_fractions)
    print_mean_and_sem(lower_levels)
    print_mean_and_sem(com_step_sizes)


if __name__ == "__main__":
    bisection_output_filepath = Path("..", "playground", "converged_bisection_move_info_p960.dat")
    com_output_filepath = Path("..", "playground", "converged_centre_of_mass_step_size_p960.dat")

    project_info_toml_filepath = Path("..", "project_info_toml_files", "local_mcmc_param_search_p960.toml")
    info = parse_project_info(project_info_toml_filepath)
    formatter = BasicProjectDirectoryFormatter()
    manager = ProjectDirectoryStructureManager(info, formatter)
    reader = ProjectDataReader(manager)

    # sim_id = int(sys.argv[1])
    # plot_monte_carlo_acceptance_rates(project_info, sim_id)
    # plot_monte_carlo_steps(project_info, sim_id)
    # converged_monte_carlo_steps(project_info, sim_id, (300, 500))

    # write_converged_bisection_multibead_position_move_info_last(project_info, bisection_output_filepath, range(31), 200)
    # write_converged_centre_of_mass_step_size_last(project_info, com_output_filepath, range(31), 200)

    # multibead_info_data = read_converged_bisection_multibead_position_move_info(bisection_output_filepath)

    # for sim_id, multibead_info in multibead_info_data:
    #     print(f"{sim_id}  :  {multibead_info.lower_level}  {multibead_info.upper_level_fraction}")

    # com_info_data = read_converged_centre_of_mass_step_size(com_output_filepath)
    # for sim_id, com_step_size in com_info_data:
    #     print(f"{sim_id}  :  {com_step_size: 12.8f}")
