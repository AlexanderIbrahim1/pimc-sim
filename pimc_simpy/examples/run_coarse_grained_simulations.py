"""
This script contains pseudocode to help me find out what I need to implement to be able
to run the jobs manager.
"""

from pathlib import Path
import subprocess
from typing import Any

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.manage import get_abs_slurm_output_filename
from pimc_simpy.manage import get_abs_simulations_job_output_dirpath
from pimc_simpy.manage import get_slurm_bashfile_filepath
from pimc_simpy.manage import get_toml_filepath
from pimc_simpy.manage import mkdir_subproject_dirpaths
from pimc_simpy.manage import mkdir_job_and_output_dirpaths

from project_info import ProjectInfo


def get_toml_file_contents(contents_map: dict[str, Any]) -> str:
    abs_output_dirpath: Path | str = contents_map["abs_output_dirpath"]
    abs_repo_dirpath: Path | str = contents_map["abs_repo_dirpath"]
    density: float = contents_map["density"]
    cell_dimensions: tuple[float, float, float] = contents_map["cell_dimensions"]
    seed: int | str = contents_map["seed"]
    last_block_index = contents_map["last_block_index"]
    n_equilibrium_blocks = contents_map["n_equilibrium_blocks"]
    n_passes = contents_map["n_passes"]
    n_timeslices = contents_map["n_timeslices"]
    centre_of_mass_step_size = contents_map["centre_of_mass_step_size"]
    bisection_level = contents_map["bisection_level"]
    bisection_ratio = contents_map["bisection_ratio"]

    contents = "\n".join(
        [
            f"abs_output_dirpath = '{str(abs_output_dirpath)}'",
            "first_block_index = 0",
            f"last_block_index = {last_block_index}",
            f"n_equilibrium_blocks = {n_equilibrium_blocks}",
            f"n_passes = {n_passes}",
            f"n_timeslices = {n_timeslices}",
            f"centre_of_mass_step_size = {centre_of_mass_step_size}",
            f"bisection_level = {bisection_level}",
            f"bisection_ratio = {bisection_ratio}",
            f"density = {density:.8f}",
            "temperature = 4.2",
            f"initial_seed = {seed}",
            f"n_cells_dim0 = {cell_dimensions[0]}",
            f"n_cells_dim1 = {cell_dimensions[1]}",
            f"n_cells_dim2 = {cell_dimensions[2]}",
            f"abs_two_body_filepath =   '{str(abs_repo_dirpath)}/potentials/fsh_potential_angstroms_wavenumbers.potext_sq'",
            f"abs_three_body_filepath = '{str(abs_repo_dirpath)}/playground/scripts/threebody_126_101_51.dat'",
            f"abs_four_body_filepath =  '{str(abs_repo_dirpath)}/playground/scripts/models/fourbodypara_8_16_16_8.pt'",
        ]
    )

    return contents


def get_slurm_file_contents(contents_map: dict[str, Any]) -> str:
    abs_executable_filepath: Path | str = contents_map["abs_executable_filepath"]
    abs_toml_filepath: Path | str = contents_map["abs_toml_filepath"]
    memory_gb: int = contents_map["memory_gb"]
    abs_slurm_output_filename: str = contents_map["abs_slurm_output_filename"]

    contents = "\n".join(
        [
            "#!/bin/bash",
            "",
            "#SBATCH --account=rrg-pnroy",
            f"#SBATCH --mem={memory_gb}G",
            "#SBATCH --time=0-06:00:00",
            "#SBATCH --cpus-per-task=1",
            f"#SBATCH --output={str(abs_slurm_output_filename)}",
            "",
            f'executable="{str(abs_executable_filepath)}"',
            f'toml_file="{str(abs_toml_filepath)}"',
            "",
            "${executable} ${toml_file}",
            "",
        ]
    )

    return contents


def example(densities: NDArray) -> None:
    info = ProjectInfo()

    toml_info_map: dict[str, Any] = {}
    toml_info_map["abs_repo_dirpath"] = info.abs_repo_dirpath
    toml_info_map["cell_dimensions"] = (3, 2, 2)
    toml_info_map["seed"] = '"RANDOM"'
    toml_info_map["last_block_index"] = 1000
    toml_info_map["n_equilibrium_blocks"] = 1000
    toml_info_map["n_passes"] = 20
    toml_info_map["n_timeslices"] = 64
    toml_info_map["centre_of_mass_step_size"] = 0.3
    toml_info_map["bisection_level"] = 3
    toml_info_map["bisection_ratio"] = 0.5

    slurm_info_map: dict[str, Any] = {}
    slurm_info_map["abs_executable_filepath"] = info.abs_executable_filepath
    slurm_info_map["memory_gb"] = 4

    mkdir_subproject_dirpaths(info)

    for sim_id, density in enumerate(densities):
        # create the locations for the simulation and the output
        mkdir_job_and_output_dirpaths(info, sim_id)

        # create the toml file
        toml_info_map["abs_output_dirpath"] = get_abs_simulations_job_output_dirpath(info, sim_id)
        toml_info_map["density"] = density

        toml_file_contents = get_toml_file_contents(toml_info_map)
        abs_toml_filepath = get_toml_filepath(info, sim_id)
        with open(abs_toml_filepath, "w") as fout:
            fout.write(toml_file_contents)

        # create the slurm file (in a separate directory?)
        slurm_info_map["abs_toml_filepath"] = abs_toml_filepath
        slurm_info_map["abs_slurm_output_filename"] = get_abs_slurm_output_filename(info, sim_id)

        slurm_file_contents = get_slurm_file_contents(slurm_info_map)
        abs_slurm_filepath = get_slurm_bashfile_filepath(info, sim_id)
        with open(abs_slurm_filepath, "w") as fout:
            fout.write(slurm_file_contents)


def run_slurm_files(densities: NDArray) -> None:
    info = ProjectInfo()

    for sim_id in range(1, len(densities)):
        abs_slurm_filepath = get_slurm_bashfile_filepath(info, sim_id)

        cmd = ["sbatch", str(abs_slurm_filepath)]
        subprocess.run(cmd, check=True)


if __name__ == "__main__":
    n_densities = 31
    densities = np.linspace(0.024, 0.1, n_densities)  # ANG^{-3}

    # example(densities)
    run_slurm_files(densities)
