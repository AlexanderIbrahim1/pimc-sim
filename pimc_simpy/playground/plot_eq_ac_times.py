"""
This script plots certain estimated properties collected from the Monte Carlo simulations
to find the equilbration and autocorrelation times.
"""

from pathlib import Path

from project_info import ProjectInfo

from pimc_simpy.data import read_property_data
from pimc_simpy.manage import get_abs_job_output_dirpath
from pimc_simpy.manage import SimulationProjectInfo
from pimc_simpy.plotting import plot_property_rescaled


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


def plot_energies(sim_id: int) -> None:
    info = ProjectInfo()

    potential_filepath = get_abs_job_output_dirpath(info, sim_id) / POTENTIAL_ENERGY_FILENAME
    kinetic_filepath = get_abs_job_output_dirpath(info, sim_id) / KINETIC_ENERGY_FILENAME

    potential_data = read_property_data(potential_filepath)
    kinetic_data = read_property_data(kinetic_filepath)

    plot_property_rescaled([potential_data, kinetic_data], labels=["potential", "kinetic"])


if __name__ == "__main__":
    sim_id = 30
    plot_energies(sim_id)
