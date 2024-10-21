"""
This script plots certain estimated properties collected from the Monte Carlo simulations
to find the equilbration and autocorrelation times.
"""

import math
import sys
from pathlib import Path

from pimc_simpy.data import between_epochs
from pimc_simpy.quick_analysis import read_project_pair_potential_energy
from pimc_simpy.quick_analysis import read_project_kinetic_energy
from pimc_simpy.manage import ProjectInfo
from pimc_simpy.manage import parse_project_info
from pimc_simpy.plotting import plot_property_rescaled
from pimc_simpy.statistics import autocorrelation_time_from_data


def plot_energies_rescaled(info: ProjectInfo, sim_id: int) -> None:
    pair_potential_energies = read_project_pair_potential_energy(info, sim_id)
    kinetic_energies = read_project_kinetic_energy(info, sim_id)
    plot_property_rescaled([pair_potential_energies, kinetic_energies], labels=["potential", "kinetic"])


def calculate_autocorrelation(info: ProjectInfo, sim_id: int, n_equilibration_sweeps: int) -> None:
    potential_data = read_project_pair_potential_energy(info, sim_id)
    kinetic_data = read_project_kinetic_energy(info, sim_id)

    potential_data = between_epochs(n_equilibration_sweeps, potential_data.values.size, potential_data)
    kinetic_data = between_epochs(n_equilibration_sweeps, kinetic_data.values.size, kinetic_data)

    potential_auto_time = autocorrelation_time_from_data(potential_data.values)
    kinetic_auto_time = autocorrelation_time_from_data(kinetic_data.values)
    n_auto_sweeps = math.ceil(max(potential_auto_time, kinetic_auto_time))

    print(f"potential_auto_time = {potential_auto_time}")
    print(f"kinetic_auto_time = {kinetic_auto_time}")
    print(f"n_autocorrelation_sweeps = {n_auto_sweeps}")


if __name__ == "__main__":
    project_info_toml_filepath = Path("..", "project_info_toml_files", "local_mcmc_param_search_p64.toml")
    project_info = parse_project_info(project_info_toml_filepath)

    sim_id = int(sys.argv[1])
    # plot_energies(info, sim_id)
    for sim_id in range(31):
        print(f"sim_id = {sim_id}")
        calculate_autocorrelation(project_info, sim_id, 500)
