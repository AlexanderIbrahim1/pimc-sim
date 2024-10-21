"""
This script contains code for collecting information about the optimal Monte Carlo
step size information from the simulations run so far.
"""

import sys
from pathlib import Path
from typing import Sequence

from pimc_simpy.data import PropertyData
from pimc_simpy.data import last_n_epochs
from pimc_simpy.data import get_property_statistics
from pimc_simpy.quick_analysis import read_project_bisection_multibead_position_move_acceptance
from pimc_simpy.quick_analysis import read_project_single_bead_position_move_acceptance
from pimc_simpy.quick_analysis import read_project_centre_of_mass_position_move_acceptance
from pimc_simpy.quick_analysis import read_project_bisection_multibead_position_move_info
from pimc_simpy.quick_analysis import read_project_centre_of_mass_step_size
from pimc_simpy.manage import ProjectInfo
from pimc_simpy.manage import parse_project_info
from pimc_simpy.plotting import plot_property


def plot_monte_carlo_acceptance_rates(info: ProjectInfo, sim_id: int) -> None:
    bisect_accepts, bisect_rejects = read_project_bisection_multibead_position_move_acceptance(info, sim_id)
    single_accepts, single_rejects = read_project_single_bead_position_move_acceptance(info, sim_id)
    centre_accepts, centre_rejects = read_project_centre_of_mass_position_move_acceptance(info, sim_id)
    # upper_fractions, lower_levels = read_project_bisection_multibead_position_move_info(info, sim_id)
    # com_step_sizes = read_project_centre_of_mass_step_size(info, sim_id)

    bisect_accept_rates = bisect_accepts / (bisect_accepts + bisect_rejects)
    single_accept_rates = single_accepts / (single_accepts + single_rejects)
    centre_accept_rates = centre_accepts / (centre_accepts + centre_rejects)

    plot_property([bisect_accept_rates, single_accept_rates, centre_accept_rates], labels=["bisect", "single", "centre"])


def plot_monte_carlo_steps(info: ProjectInfo, sim_id: int) -> None:
    upper_fractions, lower_levels = read_project_bisection_multibead_position_move_info(info, sim_id)
    com_step_sizes = read_project_centre_of_mass_step_size(info, sim_id)

    plot_property([upper_fractions, lower_levels, com_step_sizes], labels=["upper_fractions", "lower_levels", "com_step_sizes"])


def converged_monte_carlo_steps(info: ProjectInfo, sim_id: int, n_last: int) -> None:
    upper_fractions, lower_levels = read_project_bisection_multibead_position_move_info(info, sim_id)
    com_step_sizes = read_project_centre_of_mass_step_size(info, sim_id)

    def print_mean_and_sem(data: PropertyData) -> None:
        data = last_n_epochs(n_last, data)
        stats = get_property_statistics(data)

        print(f"(mean, sem) = ({stats.mean}, {stats.stderrmean})")

    print_mean_and_sem(upper_fractions)
    print_mean_and_sem(lower_levels)
    print_mean_and_sem(com_step_sizes)


def write_converged_monte_carlo_steps(info: ProjectInfo, n_last: int, output_filepath: Path, sim_ids: Sequence[int]) -> None:
    def get_mean(data: PropertyData) -> float:
        data = last_n_epochs(n_last, data)
        stats = get_property_statistics(data)
        return stats.mean

    with open(output_filepath, "w") as fout:
        for sim_id in sim_ids:
            upper_fractions, lower_levels = read_project_bisection_multibead_position_move_info(info, sim_id)
            com_step_sizes = read_project_centre_of_mass_step_size(info, sim_id)

            upper_fraction_mean = get_mean(upper_fractions)
            lower_level_mean = round(get_mean(lower_levels))
            com_step_size_mean = get_mean(com_step_sizes)

            fout.write(f"{sim_id:>2d}  {upper_fraction_mean: 12.8f}  {lower_level_mean:>2d}  {com_step_size_mean: 12.8f}\n")


if __name__ == "__main__":
    project_info_toml_filepath = Path("..", "project_info_toml_files", "local_mcmc_param_search_p64.toml")
    project_info = parse_project_info(project_info_toml_filepath)
    output_filepath = Path(".", "example_output.dat")

    sim_id = int(sys.argv[1])
    # plot_monte_carlo_acceptance_rates(project_info, sim_id)
    # plot_monte_carlo_steps(project_info, sim_id)
    # converged_monte_carlo_steps(project_info, sim_id, 200)

    write_converged_monte_carlo_steps(project_info, 200, output_filepath, range(31))
