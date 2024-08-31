"""
This script contains pseudocode to help me find out what I need to implement to be able
to run the jobs manager.
"""

from pathlib import Path
from typing import Any

import numpy as np


def format_sim_id(sim_id: int) -> str:
    """
    Create a specific function to format the simulation ID since it is used in so many
    different places in this script.
    """
    return f"{sim_id:0>3d}"


def get_simulation_dirpath(abs_sim_root_dirpath: Path | str, sim_id: int) -> Path:
    return Path(
        f"{str(abs_sim_root_dirpath)}", "playground", "many_simulations_test", f"simulation_{format_sim_id(sim_id)}"
    )


def get_simulation_output_dirpath(abs_sim_dirpath: Path | str) -> Path:
    return Path(abs_sim_dirpath) / "output"


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
    abs_slurm_output_prefix: Path | str = contents_map["abs_slurm_output_prefix"]
    sim_id: int = contents_map["sim_id"]
    memory_gb: int = contents_map["memory_gb"]

    abs_slurm_output_filepath = f"{abs_slurm_output_prefix}-{format_sim_id(sim_id)}-%j.out"
    contents = "\n".join(
        [
            "#!/bin/bash",
            "",
            "#SBATCH --account=rrg-pnroy",
            f"#SBATCH --mem={memory_gb}G",
            "#SBATCH --time=0-06:00:00",
            "#SBATCH --cpus-per-task=1",
            f"#SBATCH --output={str(abs_slurm_output_filepath)}",
            "",
            f"{str(abs_executable_filepath)}  {str(abs_toml_filepath)}",
        ]
    )

    return contents


def get_slurm_bash_filename(sim_id: int) -> str:
    return f"slurm_{format_sim_id(sim_id)}.sh"


def example() -> None:
    n_densities = 31
    densities = np.linspace(0.024, 0.1, n_densities)  # ANG^{-3}

    abs_repo_dirpath = Path("/home/a68ibrah/research/simulations/pimc-sim")
    abs_executable_filepath = abs_repo_dirpath / "build" / "dev" / "source" / "pimc-sim"
    abs_slurm_bash_dirpath = abs_repo_dirpath / "pimc_simpy" / "playground" / "slurm_files"
    abs_slurm_output_dirpath = abs_slurm_bash_dirpath / "slurm_output"
    abs_slurm_output_prefix = str(abs_slurm_output_dirpath / "slurm")
    toml_filename = "sim.toml"  # okay to have it be the same for each simulation; makes commands simpler
    memory_gb = 4

    toml_file_contents_map: dict[str, Any] = {}
    toml_file_contents_map["abs_repo_dirpath"] = abs_repo_dirpath
    toml_file_contents_map["cell_dimensions"] = (5, 3, 3)
    toml_file_contents_map["seed"] = '"RANDOM"'
    toml_file_contents_map["last_block_index"] = 500
    toml_file_contents_map["n_equilibrium_blocks"] = 20
    toml_file_contents_map["n_passes"] = 1
    toml_file_contents_map["n_timeslices"] = 64
    toml_file_contents_map["centre_of_mass_step_size"] = 0.3
    toml_file_contents_map["bisection_level"] = 3
    toml_file_contents_map["bisection_ratio"] = 0.5

    slurm_file_contents_map: dict[str, Any] = {}
    slurm_file_contents_map["abs_executable_filepath"] = abs_executable_filepath
    slurm_file_contents_map["abs_slurm_output_prefix"] = abs_slurm_output_prefix
    slurm_file_contents_map["memory_gb"] = memory_gb

    abs_slurm_bash_dirpath.mkdir()
    abs_slurm_output_dirpath.mkdir()

    for sim_id, density in enumerate(densities):
        # create the locations for the simulation and the output
        abs_sim_dirpath = get_simulation_dirpath(abs_repo_dirpath, sim_id)
        abs_output_dirpath = get_simulation_output_dirpath(abs_sim_dirpath)

        abs_sim_dirpath.mkdir()
        abs_output_dirpath.mkdir()

        # create the toml file
        toml_file_contents_map["abs_output_dirpath"] = abs_output_dirpath
        toml_file_contents_map["density"] = density

        toml_contents = get_toml_file_contents(toml_file_contents_map)
        abs_toml_filepath = abs_sim_dirpath / toml_filename
        with open(abs_toml_filepath, "w") as fout:
            fout.write(toml_contents)

        # create the slurm file (in a separate directory?)
        slurm_file_contents_map["abs_toml_filepath"] = abs_toml_filepath
        slurm_file_contents_map["sim_id"] = sim_id

        slurm_contents = get_slurm_file_contents(slurm_file_contents_map)
        slurm_filename = get_slurm_bash_filename(sim_id)
        abs_slurm_filepath = abs_slurm_bash_dirpath / slurm_filename
        with open(abs_slurm_filepath, "w") as fout:
            fout.write(slurm_contents)


if __name__ == "__main__":
    example()
