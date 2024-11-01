"""
This script plots certain estimated properties collected from the Monte Carlo simulations
to find the equilbration and autocorrelation times.
"""

import math
from pathlib import Path

from pimc_simpy.data import between_epochs
from pimc_simpy.data import read_property_data
from pimc_simpy.data import PropertyData

from pimc_simpy.quick_analysis import ProjectDataReader

from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager
from pimc_simpy.manage import parse_project_info

from pimc_simpy.plotting import plot_property_rescaled

from pimc_simpy.statistics import autocorrelation_time_from_data


def plot_energies_rescaled(
    kinetic: PropertyData,
    pair_potential: PropertyData,
    triplet_potential: PropertyData,
) -> None:
    plot_property_rescaled([kinetic, pair_potential, triplet_potential], labels=["kinetic", "pair", "triplet"])


def calculate_autocorrelation(
    kinetic: PropertyData,
    pair_potential: PropertyData,
    triplet_potential: PropertyData,
    n_equilibration_sweeps: int,
) -> None:
    pair_data = between_epochs(n_equilibration_sweeps, pair_potential.values.size, pair_potential)
    triplet_data = between_epochs(n_equilibration_sweeps, triplet_potential.values.size, triplet_potential)
    kinetic_data = between_epochs(n_equilibration_sweeps, kinetic.values.size, kinetic)

    pair_auto_time = autocorrelation_time_from_data(pair_data.values)
    triplet_auto_time = autocorrelation_time_from_data(triplet_data.values)
    kinetic_auto_time = autocorrelation_time_from_data(kinetic_data.values)
    n_auto_sweeps = math.ceil(max(pair_auto_time, triplet_auto_time, kinetic_auto_time))

    # print(f"potential_auto_time = {potential_auto_time}")
    # print(f"kinetic_auto_time = {kinetic_auto_time}")
    print(f"n_autocorrelation_sweeps = {n_auto_sweeps}")


if __name__ == "__main__":
    n_timeslices = 192
    n_particles = 3 * 2 * 2 * 4
    eq_ac_sims_dirpath = Path("..", "..", "playground", "eq_ac_sims", f"p{n_timeslices:0>3d}")

    kinetic_filepath = eq_ac_sims_dirpath / "kinetic.dat"
    pair_potential_filepath = eq_ac_sims_dirpath / "pair_potential.dat"
    triplet_potential_filepath = eq_ac_sims_dirpath / "triplet_potential.dat"

    kinetic = read_property_data(kinetic_filepath, normalize_by=n_particles)
    pair_potential = read_property_data(pair_potential_filepath, normalize_by=n_particles)
    triplet_potential = read_property_data(triplet_potential_filepath, normalize_by=n_particles)

    # plot_energies_rescaled(kinetic, pair_potential, triplet_potential)
    calculate_autocorrelation(kinetic, pair_potential, triplet_potential, 10)
