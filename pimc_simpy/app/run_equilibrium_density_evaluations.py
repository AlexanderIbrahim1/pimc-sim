"""
This script contains pseudocode to help me find out what I need to implement to be able
to run the jobs manager.
"""

from pathlib import Path
from typing import Any
import subprocess

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager

from pimc_simpy.manage import parse_project_info


def number_of_worldline_files(n_timeslices: int) -> int:
    n_timeslices_max = 960
    return n_timeslices_max // n_timeslices


def format_block_indices(block_indices: list[int]) -> str:
    comma_sep_indices = ", ".join([str(i) for i in block_indices])
    return f"[{comma_sep_indices}]"


def get_toml_file_contents(contents_map: dict[str, Any]) -> str:
    abs_output_dirpath: Path | str = contents_map["abs_output_dirpath"]
    abs_worldlines_dirpath: Path | str = contents_map["abs_worldlines_dirpath"]
    block_indices: list[int] = contents_map["block_indices"]
    abs_repo_dirpath: Path | str = contents_map["abs_repo_dirpath"]
    density: float = contents_map["density"]
    cell_dimensions: tuple[float, float, float] = contents_map["cell_dimensions"]
    evaluate_two_body: str = contents_map["evaluate_two_body"]
    evaluate_three_body: str = contents_map["evaluate_three_body"]
    evaluate_four_body: str = contents_map["evaluate_four_body"]

    formatted_block_indices = format_block_indices(block_indices)

    contents = "\n".join(
        [
            f"abs_output_dirpath = '{str(abs_output_dirpath)}'",
            f"abs_worldlines_dirpath = '{str(abs_worldlines_dirpath)}'",
            f"block_indices = {formatted_block_indices}",
            f"density = {density:.8f}",
            f"n_cells_dim0 = {cell_dimensions[0]}",
            f"n_cells_dim1 = {cell_dimensions[1]}",
            f"n_cells_dim2 = {cell_dimensions[2]}",
            f"evaluate_two_body = {evaluate_two_body}",
            f"evaluate_three_body = {evaluate_three_body}",
            f"evaluate_four_body = {evaluate_four_body}",
            f"abs_two_body_filepath =   '{str(abs_repo_dirpath)}/potentials/fsh_potential_angstroms_wavenumbers.potext_sq'",
            f"abs_three_body_filepath = '{str(abs_repo_dirpath)}/../../large_files/eng.tri'",
            f"abs_four_body_filepath =  '{str(abs_repo_dirpath)}/../../large_files/published_fourbody_torch_models/fourbodypara_ssp_64_128_128_64_cpu_eval.pt'",
        ]
    )
    # f"abs_three_body_filepath = '{str(abs_repo_dirpath)}/pimc_simpy/scripts/pes_files/threebody_126_101_51.dat'",
    # f"abs_four_body_filepath =  '{str(abs_repo_dirpath)}/pimc_simpy/scripts/models/fourbodypara_ssp_64_128_128_64_cpu_eval.pt'",

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
            "#SBATCH --time=1-00:00:00",
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


def create_directory_structure(
    sim_manager: ProjectDirectoryStructureManager,
    eval_manager: ProjectDirectoryStructureManager,
    densities: NDArray,
    worldline_indices: list[int],
) -> None:
    toml_info_map: dict[str, Any] = {}
    toml_info_map["abs_repo_dirpath"] = eval_manager.info.abs_external_dirpath
    toml_info_map["cell_dimensions"] = (5, 3, 3)
    toml_info_map["evaluate_two_body"] = "false"
    toml_info_map["evaluate_three_body"] = "false"
    toml_info_map["evaluate_four_body"] = "true"
    toml_info_map["block_indices"] = worldline_indices

    slurm_info_map: dict[str, Any] = {}
    slurm_info_map["executable"] = "evaluate-worldline"
    slurm_info_map["abs_executable_dirpath"] = eval_manager.info.abs_executable_dirpath
    slurm_info_map["memory_gb"] = 4

    eval_manager.mkdir_subproject_dirpaths()

    for sim_id, density in enumerate(densities):
        toml_info_map["abs_worldlines_dirpath"] = sim_manager.get_abs_simulations_job_output_dirpath(sim_id)
        toml_info_map["density"] = density

        # create the locations for the simulation and the output
        eval_manager.mkdir_job_and_output_dirpaths(sim_id)

        # create the toml file
        toml_info_map["abs_output_dirpath"] = eval_manager.get_abs_simulations_job_output_dirpath(sim_id)
        toml_file_contents = get_toml_file_contents(toml_info_map)
        abs_toml_filepath = eval_manager.get_toml_filepath(sim_id)
        with open(abs_toml_filepath, "w") as fout:
            fout.write(toml_file_contents)

        # create the slurm file (in a separate directory?)
        slurm_info_map["abs_toml_filepath"] = abs_toml_filepath
        slurm_info_map["abs_slurm_output_filename"] = eval_manager.get_abs_slurm_output_filename(sim_id)

        slurm_file_contents = get_slurm_file_contents(slurm_info_map)
        abs_slurm_filepath = eval_manager.get_slurm_bashfile_filepath(sim_id)
        with open(abs_slurm_filepath, "w") as fout:
            fout.write(slurm_file_contents)


def run_slurm_files(eval_manager: ProjectDirectoryStructureManager, n_densities: int) -> None:
    for sim_id in range(n_densities):
        abs_slurm_filepath = eval_manager.get_slurm_bashfile_filepath(sim_id)

        cmd = ["sbatch", str(abs_slurm_filepath)]
        subprocess.run(cmd, check=True)


# def create_worldline_indices_map(i_start: int, gap_size: int) -> dict[int, list[int]]:
#     def indices(n_timeslices: int) -> list[int]:
#         n_worldlines = number_of_worldline_files(n_timeslices)
#         start = i_start
#         stop = 1 + i_start + gap_size * n_worldlines
#         step = gap_size
# 
#         return list(range(start, stop, step))
# 
#     worldline_indices_map: dict[int, list[int]] = {
#         64: indices(64),
#         80: indices(80),
#         96: indices(96),
#         128: indices(128),
#         192: indices(192),
#     }
# 
#     return worldline_indices_map


def get_worldline_indices_batch(n_timeslices: int, all_worldline_indices: list[int], i_batch: int) -> list[int]:
    n_worldlines = number_of_worldline_files(n_timeslices)
    i_start = i_batch * n_worldlines
    i_end = i_start + n_worldlines

    if (i_end >= len(all_worldline_indices)):
        raise ValueError("ERROR: requested batch goes beyond the possible indices.")

    return all_worldline_indices[i_start:i_end]


if __name__ == "__main__":
    all_worldline_indices = list(range(20, 2000, 10))
    densities = np.linspace(0.025, 0.027, 21)  # ANG^{-3}

    for n_timeslices in [64, 80, 96, 128, 192]:
        # create the simulation project manager
        sim_info_toml_filename = "pert2b3b_eq_dens_tmpl.toml"
        sim_info_toml_dirpath = Path("..", "project_info_toml_files", "equilibrium_density_files")
        sim_info_toml_filepath = sim_info_toml_dirpath / sim_info_toml_filename
        sim_info = parse_project_info(sim_info_toml_filepath)
        sim_info.abs_subproject_dirpath = sim_info.abs_subproject_dirpath / f"p{n_timeslices:0>3d}" / f"version0"
        sim_info.subproject_name = f"eq_dens_p{n_timeslices:0>3d}_version0"
        sim_formatter = BasicProjectDirectoryFormatter()
        sim_manager = ProjectDirectoryStructureManager(sim_info, sim_formatter)
    
        for i_batch in range(8):
            worldline_indices = get_worldline_indices_batch(n_timeslices, all_worldline_indices, i_batch)
        
            # create the evaluation project manager
            eval_project_info_toml_filepath = Path("..", "project_info_toml_files", "eval_eq_dens_pert2b3b_tmpl.toml")
            eval_info = parse_project_info(eval_project_info_toml_filepath)
            eval_info.abs_subproject_dirpath = eval_info.abs_subproject_dirpath / f"version{i_batch}" / f"p{n_timeslices:0>3d}"
            eval_info.subproject_name += f"_p{n_timeslices:0>3d}_v{i_batch}"
            eval_formatter = BasicProjectDirectoryFormatter()
            eval_manager = ProjectDirectoryStructureManager(eval_info, eval_formatter)
        
            # create_directory_structure(sim_manager, eval_manager, densities, worldline_indices)
            run_slurm_files(eval_manager, len(densities))


