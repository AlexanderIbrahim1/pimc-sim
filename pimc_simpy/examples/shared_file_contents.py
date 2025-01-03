"""
This script contains code to run identical simulations, but each with a different
worldline file as a starting seed.
"""

from pathlib import Path
from typing import Any


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
    writer_batch_size: int = contents_map["writer_batch_size"]
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
            f"writer_batch_size = {writer_batch_size}",
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

    return contents


def get_slurm_file_contents(contents_map: dict[str, Any]) -> str:
    executable: str = contents_map["executable"]
    abs_executable_dirpath: Path | str = contents_map["abs_executable_dirpath"]
    abs_toml_filepath: Path | str = contents_map["abs_toml_filepath"]
    memory_gb: int = contents_map["memory_gb"]
    abs_slurm_output_filename: str = contents_map["abs_slurm_output_filename"]
    time: str = contents_map["time"]

    contents = "\n".join(
        [
            "#!/bin/bash",
            "",
            "#SBATCH --account=rrg-pnroy",
            f"#SBATCH --mem={memory_gb}G",
            f"#SBATCH --time={time}",
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
