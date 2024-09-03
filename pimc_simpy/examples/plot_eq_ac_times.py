"""
This script plots certain estimated properties collected from the Monte Carlo simulations
to find the equilbration and autocorrelation times.
"""

import sys
from pathlib import Path

from project_info import ProjectInfo

from pimc_simpy.data import read_property_data
from pimc_simpy.data import PropertyData
from pimc_simpy.data import between_epochs
from pimc_simpy.manage import get_abs_job_output_dirpath
from pimc_simpy.manage import SimulationProjectInfo
from pimc_simpy.plotting import plot_property_rescaled
from pimc_simpy.statistics import autocorrelation_time_from_data


class ProjectInfo(SimulationProjectInfo):
    def __init__(self) -> None:
        self.abs_a68home = Path("/home/a68ibrah/research/simulations")
        self.abs_project_dirpath = self.abs_a68home / "cluster_data" / "files"

    @property
    def abs_repo_dirpath(self) -> Path:
        return self.abs_a68home / "pimc-sim"

    @property
    def abs_executable_filepath(self) -> Path:
        return self.abs_repo_dirpath / "build" / "dev-highperf" / "source" / "pimc-sim"

    @property
    def abs_subproject_dirpath(self) -> Path:
        return self.abs_project_dirpath / "p64_coarse_eq_ac_times"

    @property
    def subproject_name(self) -> str:
        return "p64_coarse_eq_ac_times"


POTENTIAL_ENERGY_FILENAME: str = "pair_potential.dat"
KINETIC_ENERGY_FILENAME: str = "kinetic.dat"


def parse_sim_id(argv: list[str]) -> int:
    if len(argv) != 2:
        raise RuntimeError("python plot_eq_ac_times.py <sim_id>")

    sim_id = int(argv[1])
    if sim_id < 0 or sim_id > 30:
        raise ValueError("sim_id must be in [0, 30], inclusive end")

    return sim_id


def get_potential_and_kinetic_data(sim_id: int) -> tuple[PropertyData, PropertyData]:
    info = ProjectInfo()

    potential_filepath = get_abs_job_output_dirpath(info, sim_id) / POTENTIAL_ENERGY_FILENAME
    kinetic_filepath = get_abs_job_output_dirpath(info, sim_id) / KINETIC_ENERGY_FILENAME

    potential_data = read_property_data(potential_filepath)
    kinetic_data = read_property_data(kinetic_filepath)

    return potential_data, kinetic_data


def plot_energies(sim_id: int) -> None:
    potential_data, kinetic_data = get_potential_and_kinetic_data(sim_id)
    plot_property_rescaled([potential_data, kinetic_data], labels=["potential", "kinetic"])


def calculate_autocorrelation(sim_id: int, n_equilibration_sweeps: int) -> None:
    potential_data, kinetic_data = get_potential_and_kinetic_data(sim_id)

    potential_data = between_epochs(n_equilibration_sweeps, potential_data.values.size, potential_data)
    kinetic_data = between_epochs(n_equilibration_sweeps, kinetic_data.values.size, kinetic_data)

    potential_auto_time = autocorrelation_time_from_data(potential_data.values)
    kinetic_auto_time = autocorrelation_time_from_data(kinetic_data.values)

    print(f"potential_auto_time = {potential_auto_time}")
    print(f"kinetic_auto_time = {kinetic_auto_time}")


if __name__ == "__main__":
    # sim_id = parse_sim_id(sys.argv)
    # plot_energies(sim_id)
    for sim_id in range(31):
        print(f"sim_id = {sim_id}")
        calculate_autocorrelation(sim_id, 500)
