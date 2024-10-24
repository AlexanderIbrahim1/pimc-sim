"""
This script contains pseudocode to help me find out what I need to implement to be able
to run the jobs manager.
"""

from pathlib import Path
from typing import Any
import dataclasses
import subprocess

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.manage import ProjectDirectoryFormatter
from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager

from pimc_simpy.manage import ProjectInfo
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


def example(info: ProjectInfo, densities: NDArray) -> None:
    toml_info_map: dict[str, Any] = {}
    toml_info_map["abs_repo_dirpath"] = info.abs_external_dirpath
    toml_info_map["abs_worldlines_dirpath"] = Path(".")  # TODO: put actual path here later
    toml_info_map["block_index"] = 25
    toml_info_map["cell_dimensions"] = (5, 3, 3)
    toml_info_map["evaluate_two_body"] = "true"
    toml_info_map["evaluate_three_body"] = "false"
    toml_info_map["evaluate_four_body"] = "false"

    slurm_info_map: dict[str, Any] = {}
    slurm_info_map["executable"] = "evaluate-worldline"
    slurm_info_map["abs_executable_dirpath"] = info.abs_executable_dirpath
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


def run_slurm_files(info: ProjectInfo, n_densities: int) -> None:
    # for sim_id in [0]:
    for sim_id in range(1, n_densities):
        abs_slurm_filepath = get_slurm_bashfile_filepath(info, sim_id)

        cmd = ["sbatch", str(abs_slurm_filepath)]
        subprocess.run(cmd, check=True)


if __name__ == "__main__":
    n_densities = 31
    densities = np.linspace(0.024, 0.1, n_densities)  # ANG^{-3}

    project_info_toml_filepath = Path("..", "project_info_toml_files", "local_eq_ac_search_p960.toml")
    info = parse_project_info(project_info_toml_filepath)
    sim_formatter = BasicProjectDirectoryFormatter()
    sim_manager = ProjectDirectoryStructureManager(info, sim_formatter)

    eval_formatter = EvaluationDirectoryFormatter()

    example(info, densities)
    # run_slurm_files(info, n_densities)
