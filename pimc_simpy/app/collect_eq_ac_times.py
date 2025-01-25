"""
This script plots certain estimated properties collected from the Monte Carlo simulations
to find the equilbration and autocorrelation times.
"""

import math
import sys
from pathlib import Path

from pimc_simpy.data import between_epochs
from pimc_simpy.quick_analysis import ProjectDataReader
from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager
from pimc_simpy.manage import parse_project_info
from pimc_simpy.plotting import plot_property_rescaled
from pimc_simpy.statistics import autocorrelation_time_from_data


def plot_energies_rescaled(reader: ProjectDataReader, sim_id: int) -> None:
    pair_potential_energies = reader.read_project_pair_potential_energy(sim_id)
    triplet_potential_energies = reader.read_project_triplet_potential_energy(sim_id)
    kinetic_energies = reader.read_project_kinetic_energy(sim_id)
    plot_property_rescaled(
        [pair_potential_energies, triplet_potential_energies, kinetic_energies],
        labels=["pair", "triplet", "kinetic"],
    )


def calculate_autocorrelation(reader: ProjectDataReader, sim_id: int, n_equilibration_sweeps: int) -> None:
    pair_potential_data = reader.read_project_pair_potential_energy(sim_id)
    triplet_potential_data = reader.read_project_triplet_potential_energy(sim_id)
    kinetic_data = reader.read_project_kinetic_energy(sim_id)

    pair_potential_data = between_epochs(n_equilibration_sweeps, pair_potential_data.values.size, pair_potential_data)
    triplet_potential_data = between_epochs(n_equilibration_sweeps, triplet_potential_data.values.size, triplet_potential_data)
    kinetic_data = between_epochs(n_equilibration_sweeps, kinetic_data.values.size, kinetic_data)

    pair_potential_auto_time = autocorrelation_time_from_data(pair_potential_data.values)
    triplet_potential_auto_time = autocorrelation_time_from_data(triplet_potential_data.values)
    kinetic_auto_time = autocorrelation_time_from_data(kinetic_data.values)
    n_auto_sweeps = math.ceil(max(pair_potential_auto_time, triplet_potential_auto_time, kinetic_auto_time))

    print(kinetic_auto_time)
    # print(f"pair_potential_auto_time = {pair_potential_auto_time}")
    # print(f"triplet_potential_auto_time = {triplet_potential_auto_time}")
    # print(f"kinetic_auto_time = {kinetic_auto_time}")
    # print(f"n_autocorrelation_sweeps = {n_auto_sweeps}")


if __name__ == "__main__":
    project_info_filepath = Path("..", "playground", "cedar_files", "pert2b3b_p64_coarse_double", "pert2b3b_p64_coarse.toml")
    info = parse_project_info(project_info_filepath)
    formatter = BasicProjectDirectoryFormatter()
    manager = ProjectDirectoryStructureManager(info, formatter)
    reader = ProjectDataReader(manager)

    # sim_id = int(sys.argv[1])
    # plot_energies_rescaled(reader, sim_id)
    # calculate_autocorrelation(reader, sim_id, 200)

    for sim_id in range(31):
        # print(f"sim_id = {sim_id}")
        calculate_autocorrelation(reader, sim_id, 200)
