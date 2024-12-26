"""
This script runs the four-body total energy calculations for the P = 64 coarse-grained
simulations.
"""

from pathlib import Path
from typing import Any
from typing import Sequence
import dataclasses
import itertools
import re
import subprocess

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.manage import ProjectDirectoryFormatter
from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager
from pimc_simpy.manage import parse_project_info

# PLAN
# - each evaluation job picks:
#   - one of pert2b or pert2b3b
#   - one of the density indices
#   - a list of worldline indices
#
# ~/pimc_simulations/simulations/twothreefour_body/p64_coarse/pert2b/simulations/job_000/output/
# ~/pimc_simulations/evaluations/twothreefour_body/p64_coarse/pert2b/dens000


@dataclasses.dataclass
class WorldlineIndicesInfo:
    start: int
    stop: int
    step: int

    def unpack(self) -> tuple[int, int, int]:
        return (self.start, self.stop, self.step)


def worldline_indices_window(i_window: int, window_size: int, indices_info: WorldlineIndicesInfo) -> list[int]:
    i_left: int = i_window * window_size
    i_right: int = i_left + window_size

    start, stop, step = indices_info.unpack()

    i_w_start = start + i_left * step
    i_w_stop = start + i_right * step

    if i_w_start < start:
        raise ValueError("Invalid starting worldline index")

    if i_w_stop > stop:
        raise ValueError("Invalid stopping worldline index")

    return list(range(i_w_start, i_w_stop, step))


@dataclasses.dataclass
class WorldlineIndicesMaker:
    indices_info: WorldlineIndicesInfo
    evaluations_per_job: int

    def __call__(self, i_window: int) -> list[int]:
        return worldline_indices_window(i_window, self.evaluations_per_job, self.indices_info)


@dataclasses.dataclass
class EvaluationID:
    sim_id: int
    window_id: int


class EvaluationDirectoryFormatter(ProjectDirectoryFormatter):
    def format_sim_id(self, sim_id: EvaluationID) -> str:
        return f"{sim_id.sim_id:0>3d}_{sim_id.window_id:0>3d}"


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

    block_indices_str: str = "[" + ", ".join([str(idx) for idx in block_indices]) + "]"

    contents = "\n".join(
        [
            f"abs_output_dirpath = '{str(abs_output_dirpath)}'",
            f"abs_worldlines_dirpath = '{str(abs_worldlines_dirpath)}'",
            f"block_indices = {block_indices_str}",
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


def create_directory_structure(
    sim_manager: ProjectDirectoryStructureManager,
    eval_manager: ProjectDirectoryStructureManager,
    densities: NDArray,
    i_window: int,
    worldline_indices_maker: WorldlineIndicesMaker,
) -> None:
    toml_info_map: dict[str, Any] = {}
    toml_info_map["abs_repo_dirpath"] = eval_manager.info.abs_external_dirpath
    toml_info_map["cell_dimensions"] = (5, 3, 3)
    toml_info_map["evaluate_two_body"] = "false"
    toml_info_map["evaluate_three_body"] = "false"
    toml_info_map["evaluate_four_body"] = "true"

    slurm_info_map: dict[str, Any] = {}
    slurm_info_map["executable"] = "evaluate-worldline"
    slurm_info_map["abs_executable_dirpath"] = eval_manager.info.abs_executable_dirpath
    slurm_info_map["memory_gb"] = 4

    eval_manager.mkdir_subproject_dirpaths()

    for sim_id, density in enumerate(densities):
        toml_info_map["abs_worldlines_dirpath"] = sim_manager.get_abs_simulations_job_output_dirpath(sim_id)
        toml_info_map["density"] = density

        worldline_indices = worldline_indices_maker(i_window)
        eval_id = EvaluationID(sim_id, i_window)

        # create the locations for the simulation and the output
        eval_manager.mkdir_job_and_output_dirpaths(eval_id)

        # create the toml file
        toml_info_map["block_indices"] = worldline_indices
        toml_info_map["abs_output_dirpath"] = eval_manager.get_abs_simulations_job_output_dirpath(eval_id)

        toml_file_contents = get_toml_file_contents(toml_info_map)
        abs_toml_filepath = eval_manager.get_toml_filepath(eval_id)
        with open(abs_toml_filepath, "w") as fout:
            fout.write(toml_file_contents)

        # create the slurm file (in a separate directory?)
        slurm_info_map["abs_toml_filepath"] = abs_toml_filepath
        slurm_info_map["abs_slurm_output_filename"] = eval_manager.get_abs_slurm_output_filename(eval_id)

        slurm_file_contents = get_slurm_file_contents(slurm_info_map)
        abs_slurm_filepath = eval_manager.get_slurm_bashfile_filepath(eval_id)
        with open(abs_slurm_filepath, "w") as fout:
            fout.write(slurm_file_contents)


def run_slurm_files(
    eval_manager: ProjectDirectoryStructureManager,
    n_densities: int,
    i_window: int,
) -> None:
    for sim_id in range(n_densities):
        eval_id = EvaluationID(sim_id, i_window)
        abs_slurm_filepath = eval_manager.get_slurm_bashfile_filepath(eval_id)

        cmd = ["sbatch", str(abs_slurm_filepath)]
        subprocess.run(cmd, check=True)


if __name__ == "__main__":
    indices_info = WorldlineIndicesInfo(200, 15000, 25)
    evaluations_per_job = 16
    worldline_indices_maker = WorldlineIndicesMaker(indices_info, evaluations_per_job)

    densities = np.linspace(0.024, 0.1, 31)  # ANG^{-3}

    sim_filename = "p64_coarse_pert2b3b.toml"
    sim_project_info_toml_filepath = Path("..", "project_info_toml_files", "p64_coarse", sim_filename)
    sim_info = parse_project_info(sim_project_info_toml_filepath)
    sim_formatter = BasicProjectDirectoryFormatter()
    sim_manager = ProjectDirectoryStructureManager(sim_info, sim_formatter)

    eval_filename = "p64_coarse_pert2b3b_eval.toml"
    eval_project_info_toml_filepath = Path("..", "project_info_toml_files", "p64_coarse", eval_filename)
    eval_info = parse_project_info(eval_project_info_toml_filepath)
    eval_formatter = EvaluationDirectoryFormatter()
    eval_manager = ProjectDirectoryStructureManager(eval_info, eval_formatter)

    for i_window in range(20):
        # create_directory_structure(sim_manager, eval_manager, densities, i_window, worldline_indices_maker)
        run_slurm_files(eval_manager, len(densities), i_window)
