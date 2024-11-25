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


@dataclasses.dataclass
class MCStepSizes:
    com_step_size: float
    bisection_upper_level_fraction: float
    bisection_lower_level: int


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
    save_worldlines: str = contents_map["save_worldlines"]
    n_save_worldlines_every: int = contents_map["n_save_worldlines_every"]
    freeze_mc_steps: bool = contents_map["freeze_mc_steps"]
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
            f"save_worldlines = {save_worldlines}",
            f"n_save_worldlines_every = {n_save_worldlines_every}",
            f"centre_of_mass_step_size = {centre_of_mass_step_size}",
            f"bisection_level = {bisection_level}",
            f"bisection_ratio = {bisection_ratio}",
            f"density = {density:.8f}",
            "temperature = 4.2",
            f"initial_seed = {seed}",
            f"n_cells_dim0 = {cell_dimensions[0]}",
            f"n_cells_dim1 = {cell_dimensions[1]}",
            f"n_cells_dim2 = {cell_dimensions[2]}",
            f"freeze_monte_carlo_step_sizes_in_equilibrium = {freeze_mc_steps}",
            f"abs_two_body_filepath =   '{str(abs_repo_dirpath)}/potentials/fsh_potential_angstroms_wavenumbers.potext_sq'",
            f"abs_three_body_filepath = '{str(abs_repo_dirpath)}/../../large_files/eng.tri'",
            f"abs_four_body_filepath =  '{str(abs_repo_dirpath)}/../../large_files/published_fourbody_torch_models/fourbodypara_ssp_64_128_128_64_cpu_eval.pt'",
        ]
    )
    # f"abs_three_body_filepath = '{str(abs_repo_dirpath)}/pimc_simpy/scripts/pes_files/threebody_126_101_51.dat'",

    return contents


def get_slurm_file_contents(contents_map: dict[str, Any]) -> str:
    executable: str = contents_map["executable"]
    abs_executable_dirpath: Path | str = contents_map["abs_executable_dirpath"]
    abs_toml_filepath: Path | str = contents_map["abs_toml_filepath"]
    memory_gb: int = contents_map["memory_gb"]
    abs_slurm_output_filename: str = contents_map["abs_slurm_output_filename"]

    contents = "\n".join(
        [
            "#!/bin/bash",
            "",
            "#SBATCH --account=rrg-pnroy",
            f"#SBATCH --mem={memory_gb}G",
            "#SBATCH --time=2-00:00:00",
            "#SBATCH --cpus-per-task=1",
            f"#SBATCH --output={str(abs_slurm_output_filename)}",
            "",
            f'executable="{str(abs_executable_dirpath)}/{executable}"',
            f'toml_file="{str(abs_toml_filepath)}"',
            "",
            "${executable} ${toml_file}",
            "",
        ]
    )

    return contents


def example(
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
    toml_info_map["last_block_index"] = 10000
    toml_info_map["n_equilibrium_blocks"] = 15
    toml_info_map["n_passes"] = 2
    toml_info_map["n_timeslices"] = n_timeslices
    toml_info_map["save_worldlines"] = "false"
    toml_info_map["n_save_worldlines_every"] = 1
    toml_info_map["freeze_mc_steps"] = "true"

    slurm_info_map: dict[str, Any] = {}
    slurm_info_map["executable"] = "perturbative2b"
    slurm_info_map["abs_executable_dirpath"] = manager.info.abs_executable_dirpath
    slurm_info_map["memory_gb"] = 4

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
        64: MCStepSizes(com_step_size=0.175,  bisection_upper_level_fraction=0.71, bisection_lower_level=2),
        80: MCStepSizes(com_step_size=0.175,  bisection_upper_level_fraction=0.03, bisection_lower_level=3),
        96: MCStepSizes(com_step_size=0.175,  bisection_upper_level_fraction=0.31, bisection_lower_level=3),
        128: MCStepSizes(com_step_size=0.170, bisection_upper_level_fraction=0.71, bisection_lower_level=3),
        192: MCStepSizes(com_step_size=0.170, bisection_upper_level_fraction=0.33, bisection_lower_level=4),
    }

    all_n_timeslices = [64, 80, 96, 128, 192]
    all_versions = list(range(10))

    for n_timeslices in all_n_timeslices:
        for version in all_versions:
            project_info_toml_dirpath = Path("..", "project_info_toml_files", "equilibrium_density_files")
            project_info_toml_filename = f"pert2b_eq_dens_tmpl.toml"
            project_info_toml_filepath = project_info_toml_dirpath / project_info_toml_filename
            project_info = parse_project_info(project_info_toml_filepath)
            project_info.abs_subproject_dirpath = project_info.abs_subproject_dirpath / f"version{version}" / f"p{n_timeslices:0>3d}"
            project_info.subproject_name = f"eq_dens_p{n_timeslices:0>3d}_v{version}"

            formatter = BasicProjectDirectoryFormatter()
            manager = ProjectDirectoryStructureManager(project_info, formatter)

            # example(manager, densities, n_timeslices, step_sizes_map)
            run_slurm_files(manager, n_densities)
