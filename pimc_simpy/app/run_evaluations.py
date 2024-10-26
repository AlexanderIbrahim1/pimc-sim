"""
This script contains pseudocode to help me find out what I need to implement to be able
to run the jobs manager.
"""

from pathlib import Path
from typing import Any
from typing import Sequence
import itertools
import dataclasses
import subprocess

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.manage import ProjectDirectoryFormatter
from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager

from pimc_simpy.manage import parse_project_info


@dataclasses.dataclass
class EvaluationID:
    sim_id: int
    worldline_index: int


class EvaluationDirectoryFormatter(ProjectDirectoryFormatter):
    def format_sim_id(self, eval_id: EvaluationID) -> str:
        return f"{eval_id.sim_id:0>3d}_{eval_id.worldline_index:0>5d}"


def get_toml_file_contents(contents_map: dict[str, Any]) -> str:
    abs_output_dirpath: Path | str = contents_map["abs_output_dirpath"]
    abs_worldlines_dirpath: Path | str = contents_map["abs_worldlines_dirpath"]
    block_index: int = contents_map["block_index"]
    abs_repo_dirpath: Path | str = contents_map["abs_repo_dirpath"]
    density: float = contents_map["density"]
    cell_dimensions: tuple[float, float, float] = contents_map["cell_dimensions"]
    evaluate_two_body: str = contents_map["evaluate_two_body"]
    evaluate_three_body: str = contents_map["evaluate_three_body"]
    evaluate_four_body: str = contents_map["evaluate_four_body"]

    contents = "\n".join(
        [
            f"abs_output_dirpath = '{str(abs_output_dirpath)}'",
            f"abs_worldlines_dirpath = '{str(abs_worldlines_dirpath)}'",
            f"block_index = {block_index}",
            f"density = {density:.8f}",
            f"n_cells_dim0 = {cell_dimensions[0]}",
            f"n_cells_dim1 = {cell_dimensions[1]}",
            f"n_cells_dim2 = {cell_dimensions[2]}",
            f"evaluate_two_body = {evaluate_two_body}",
            f"evaluate_three_body = {evaluate_three_body}",
            f"evaluate_four_body = {evaluate_four_body}",
            f"abs_two_body_filepath =   '{str(abs_repo_dirpath)}/potentials/fsh_potential_angstroms_wavenumbers.potext_sq'",
            f"abs_three_body_filepath = '{str(abs_repo_dirpath)}/../../large_files/eng.tri'",
            f"abs_four_body_filepath =  '{str(abs_repo_dirpath)}/pimc_simpy/scripts/models/fourbodypara_8_16_16_8.pt'",
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
    worldline_indices: Sequence[int],
) -> None:
    toml_info_map: dict[str, Any] = {}
    toml_info_map["abs_repo_dirpath"] = eval_manager.info.abs_external_dirpath
    toml_info_map["cell_dimensions"] = (5, 3, 3)
    toml_info_map["evaluate_two_body"] = "true"
    toml_info_map["evaluate_three_body"] = "false"
    toml_info_map["evaluate_four_body"] = "false"

    slurm_info_map: dict[str, Any] = {}
    slurm_info_map["executable"] = "evaluate-worldline"
    slurm_info_map["abs_executable_dirpath"] = eval_manager.info.abs_executable_dirpath
    slurm_info_map["memory_gb"] = 4

    eval_manager.mkdir_subproject_dirpaths()

    for sim_id, density in enumerate(densities):
        toml_info_map["abs_worldlines_dirpath"] = sim_manager.get_abs_simulations_job_output_dirpath(sim_id)
        toml_info_map["density"] = density

        for i_worldline in worldline_indices:
            eval_id = EvaluationID(sim_id, i_worldline)

            # create the locations for the simulation and the output
            eval_manager.mkdir_job_and_output_dirpaths(eval_id)

            # create the toml file
            toml_info_map["block_index"] = i_worldline
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
    worldline_indices: Sequence[int],
) -> None:
    for sim_id, i_worldline in itertools.product(range(n_densities), worldline_indices):
        eval_id = EvaluationID(sim_id, i_worldline)
        abs_slurm_filepath = eval_manager.get_slurm_bashfile_filepath(eval_id)

        cmd = ["bash", str(abs_slurm_filepath)]
        subprocess.run(cmd, check=True)
        exit()


if __name__ == "__main__":
    n_densities = 31
    densities = np.linspace(0.024, 0.1, n_densities)  # ANG^{-3}
    worldline_indices = [19 + 10 * i for i in range(32)]

    sim_project_info_toml_filepath = Path("..", "project_info_toml_files", "p960_coarse_pert2b.toml")
    sim_info = parse_project_info(sim_project_info_toml_filepath)
    sim_formatter = BasicProjectDirectoryFormatter()
    sim_manager = ProjectDirectoryStructureManager(sim_info, sim_formatter)

    eval_project_info_toml_filepath = Path("..", "project_info_toml_files", "p960_coarse_eval_pert2b_fourbody.toml")
    eval_info = parse_project_info(eval_project_info_toml_filepath)
    eval_formatter = EvaluationDirectoryFormatter()
    eval_manager = ProjectDirectoryStructureManager(eval_info, eval_formatter)

    # create_directory_structure(sim_manager, eval_manager, densities, worldline_indices)
    run_slurm_files(eval_manager, n_densities, worldline_indices)
