"""
This script reads in the estimators from completed simulations and saves the relevant data
in output files.
"""

import dataclasses
from pathlib import Path
from typing import Callable

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.data import get_property_statistics
from pimc_simpy.data import PropertyData
from pimc_simpy.manage import parse_project_info
from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager
from pimc_simpy.quick_analysis import ProjectDataReader


OUTPUT_DIRPATH = Path("/home/a68ibrah/research/simulations/pimc-sim/pimc_simpy/playground/twothreefour_body/p960_coarse_data")
QUADRUPLET_ENERGIES_DIRPATH = OUTPUT_DIRPATH / "quadruplet_energies_averaged"


@dataclasses.dataclass
class EvaluationID:
    sim_id: int
    worldline_index: int


class EvaluationDirectoryFormatter(ProjectDirectoryFormatter):
    def format_sim_id(self, eval_id: EvaluationID) -> str:
        return f"{eval_id.sim_id:0>3d}_{eval_id.worldline_index:0>5d}"


def collect_quadruplet_potential_energy(i_density: int) -> None:
    # get the project_info file for these four-body energies
    # loop over all the worldline indices
    #   - read in the projectdata
    #   - get the mean, which counts as a single estimate
    # write all these energies to a single file
    project_info_toml_filepath = Path("..", "project_info_toml_files", "local_p960_coarse_pert2b_4bevals.toml")
    info = parse_project_info(project_info_toml_filepath)
    formatter = EvaluationDirectoryFormatter()
    manager = ProjectDirectoryStructureManager(info, formatter)
    reader = ProjectDataReader(manager)

    worldline_indices = range(19, 490, 10)

    quadruplet_energies = []

    for i_worldline in worldline_indices:
        eval_id = EvaluationID(i_density, i_worldline)
        energies = reader.read_project_quadruplet_potential_energy(eval_id)
        stats = get_property_statistics(energies)
        quadruplet_energies.append(stats.mean)

    output_filename = f"quadruplet_energies_{i_density:0>3d}.txt"
    output_filepath = QUADRUPLET_ENERGIES_DIRPATH / output_filename
    np.savetxt(output_filepath, np.array(quadruplet_energies))


def write_estimator_datafile(header: str, filepath: Path, data_func: Callable[[int], PropertyData], densities: NDArray) -> None:
    with open(filepath, "w") as fout:
        fout.write(header)
        for i_sim, density in enumerate(densities):
            data = data_func(i_sim)
            stats = get_property_statistics(data)

            fout.write(f"{i_sim:>3d}   {density: 12.8f}   {stats.mean: 12.8f}   {stats.stderrmean:12.8f}\n")


def write_2b_3b_kinetic_energies() -> None:
    n_densities = 31
    n_particles = 180
    densities = np.linspace(0.024, 0.1, n_densities)

    project_info_toml_filepath = Path("..", "project_info_toml_files", "local_p960_coarse_pert2b.toml")
    info = parse_project_info(project_info_toml_filepath)
    formatter = BasicProjectDirectoryFormatter()
    manager = ProjectDirectoryStructureManager(info, formatter)
    reader = ProjectDataReader(manager)

    write_estimator_datafile(
        "# kinetic energy per particle\n# [i_sim]   [density]   [mean]    [sem]\n",
        OUTPUT_DIRPATH / "p960_coarse_kinetic_energy_data.dat",
        lambda i: reader.read_project_kinetic_energy(i) / n_particles,
        densities,
    )

    write_estimator_datafile(
        "# pair potential energy per particle\n# [i_sim]   [density]   [mean]    [sem]\n",
        OUTPUT_DIRPATH / "p960_coarse_pair_potential_energy_data.dat",
        lambda i: reader.read_project_pair_potential_energy(i) / n_particles,
        densities,
    )

    write_estimator_datafile(
        "# triplet potential energy per particle\n# [i_sim]   [density]   [mean]    [sem]\n",
        OUTPUT_DIRPATH / "p960_coarse_triplet_potential_energy_data.dat",
        lambda i: reader.read_project_triplet_potential_energy(i) / n_particles,
        densities,
    )


if __name__ == "__main__":
    for i in range(31):
        collect_quadruplet_potential_energy(i)
