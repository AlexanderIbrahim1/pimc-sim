"""
This script contains pseudocode to help me find out what I need to implement to be able
to run the jobs manager.
"""

import dataclasses
import subprocess
from pathlib import Path
from typing import Any

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager
from pimc_simpy.manage import parse_project_info

from shared_file_contents import get_toml_file_contents
from shared_file_contents import get_slurm_file_contents


@dataclasses.dataclass
class MCStepSizes:
    com_step_size: float
    bisection_upper_level_fraction: float
    bisection_lower_level: int


def make_directories(
    manager: ProjectDirectoryStructureManager, densities: NDArray, n_timeslices: int, mc_step_size_map: dict[int, MCStepSizes]
) -> None:
    toml_info_map: dict[str, Any] = {}

    mc_step_sizes = mc_step_size_map[n_timeslices]
    toml_info_map["centre_of_mass_step_size"] = mc_step_sizes.com_step_size
    toml_info_map["bisection_level"] = mc_step_sizes.bisection_lower_level
    toml_info_map["bisection_ratio"] = mc_step_sizes.bisection_upper_level_fraction

    toml_info_map["abs_repo_dirpath"] = manager.info.abs_external_dirpath
    toml_info_map["cell_dimensions"] = (5, 3, 3)
    toml_info_map["seed"] = '"RANDOM"'
    toml_info_map["last_block_index"] = 15000
    toml_info_map["n_equilibrium_blocks"] = 15
    toml_info_map["n_passes"] = 2
    toml_info_map["n_timeslices"] = n_timeslices
    toml_info_map["writer_batch_size"] = 100
    toml_info_map["save_worldlines"] = "false"
    toml_info_map["n_save_worldlines_every"] = 1
    toml_info_map["freeze_mc_steps"] = "true"

    slurm_info_map: dict[str, Any] = {}
    slurm_info_map["executable"] = "pimc-sim"
    slurm_info_map["abs_executable_dirpath"] = manager.info.abs_executable_dirpath
    slurm_info_map["memory_gb"] = 4
    slurm_info_map["time"] = "1-00:00:00"

    manager.mkdir_subproject_dirpaths()

    for sim_id, density in enumerate(densities):
        # create the locations for the simulation and the output
        manager.mkdir_job_and_output_dirpaths(sim_id)

        # create the toml file
        toml_info_map["abs_output_dirpath"] = manager.get_abs_simulations_job_output_dirpath(sim_id)
        toml_info_map["density"] = density

        toml_file_contents = get_toml_file_contents(toml_info_map)
        abs_toml_filepath = manager.get_toml_filepath(sim_id)
        with open(abs_toml_filepath, "w") as fout:
            fout.write(toml_file_contents)

        # create the slurm file (in a separate directory?)
        slurm_info_map["abs_toml_filepath"] = abs_toml_filepath
        slurm_info_map["abs_slurm_output_filename"] = manager.get_abs_slurm_output_filename(sim_id)

        slurm_file_contents = get_slurm_file_contents(slurm_info_map)
        abs_slurm_filepath = manager.get_slurm_bashfile_filepath(sim_id)
        with open(abs_slurm_filepath, "w") as fout:
            fout.write(slurm_file_contents)


def run_slurm_files(manager: ProjectDirectoryStructureManager, n_densities: int) -> None:
    for sim_id in range(n_densities):
        abs_slurm_filepath = manager.get_slurm_bashfile_filepath(sim_id)

        cmd = ["sbatch", str(abs_slurm_filepath)]
        subprocess.run(cmd, check=True)


if __name__ == "__main__":
    n_densities = 21
    densities = np.linspace(0.025, 0.027, n_densities)  # ANG^{-3}

    # NOTE: these values are essentially the same for the pert-2b and pert-2b3b simulations
    step_sizes_map = {
        64: MCStepSizes(com_step_size=0.175, bisection_upper_level_fraction=0.71, bisection_lower_level=2),
        80: MCStepSizes(com_step_size=0.175, bisection_upper_level_fraction=0.03, bisection_lower_level=3),
        96: MCStepSizes(com_step_size=0.175, bisection_upper_level_fraction=0.31, bisection_lower_level=3),
        128: MCStepSizes(com_step_size=0.170, bisection_upper_level_fraction=0.71, bisection_lower_level=3),
        192: MCStepSizes(com_step_size=0.170, bisection_upper_level_fraction=0.33, bisection_lower_level=4),
    }

    all_n_timeslices = [64, 80, 96, 128, 192]
    all_versions = list(range(10))

    for n_timeslices in all_n_timeslices:
        for version in all_versions:
            project_info_toml_dirpath = Path("..", "project_info_toml_files", "equilibrium_density_files")
            project_info_toml_filename = f"pert2b3b_double_eq_dens_tmpl.toml"
            project_info_toml_filepath = project_info_toml_dirpath / project_info_toml_filename
            project_info = parse_project_info(project_info_toml_filepath)
            project_info.abs_subproject_dirpath = (
                project_info.abs_subproject_dirpath / f"version{version}" / f"p{n_timeslices:0>3d}"
            )
            project_info.subproject_name = f"eq_dens_p{n_timeslices:0>3d}_v{version}"

            formatter = BasicProjectDirectoryFormatter()
            manager = ProjectDirectoryStructureManager(project_info, formatter)

            # make_directories(manager, densities, n_timeslices, step_sizes_map)
            run_slurm_files(manager, n_densities)
