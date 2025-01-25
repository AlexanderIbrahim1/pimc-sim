"""
This script contains pseudocode to help me find out what I need to implement to be able
to run the jobs manager.
"""

from pathlib import Path
import subprocess
from typing import Any

from numpy.typing import NDArray

from pimc_simpy.data import MultibeadPositionMoveInfo
from pimc_simpy.quick_analysis import read_converged_bisection_multibead_position_move_info
from pimc_simpy.quick_analysis import read_converged_centre_of_mass_step_size

from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager

from pimc_simpy.manage import parse_project_info

from shared_file_contents import get_toml_file_contents
from shared_file_contents import get_slurm_file_contents


def make_directories(
    manager: ProjectDirectoryStructureManager,
    densities: NDArray,
    bisection_moves: dict[int, MultibeadPositionMoveInfo],
    com_moves: dict[int, float],
) -> None:
    toml_info_map: dict[str, Any] = {}
    toml_info_map["abs_repo_dirpath"] = manager.info.abs_external_dirpath
    toml_info_map["cell_dimensions"] = (3, 2, 2)
    toml_info_map["seed"] = '"RANDOM"'
    toml_info_map["last_block_index"] = 1000
    toml_info_map["n_equilibrium_blocks"] = 0
    toml_info_map["n_passes"] = 1
    toml_info_map["n_timeslices"] = 960
    toml_info_map["writer_batch_size"] = 1
    toml_info_map["save_worldlines"] = "true"
    toml_info_map["n_save_worldlines_every"] = 1
    toml_info_map["freeze_mc_steps"] = "true"

    slurm_info_map: dict[str, Any] = {}
    slurm_info_map["executable"] = "perturbative2b3b4b"
    slurm_info_map["abs_executable_dirpath"] = manager.info.abs_executable_dirpath
    slurm_info_map["memory_gb"] = 4
    slurm_info_map["time"] = "2-00:00:00"

    manager.mkdir_subproject_dirpaths()

    for sim_id, density in enumerate(densities):
        # set the converged monte carlo step sizes
        toml_info_map["centre_of_mass_step_size"] = com_moves[sim_id]
        toml_info_map["bisection_level"] = bisection_moves[sim_id].lower_level
        toml_info_map["bisection_ratio"] = bisection_moves[sim_id].upper_level_fraction

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


# if __name__ == "__main__":
#     n_densities = 31
#     densities = np.linspace(0.024, 0.1, n_densities)  # ANG^{-3}
#
#     # bisect_info_filepath = Path("..", "playground", "converged_bisection_move_info_p960.dat")
#     # com_info_filepath = Path("..", "playground", "converged_centre_of_mass_step_size_p960.dat")
#     # bisection_moves = read_converged_bisection_multibead_position_move_info(bisect_info_filepath)
#     # com_moves = read_converged_centre_of_mass_step_size(com_info_filepath)
#
#     bisection_moves = dict([
#         (i, MultibeadPositionMoveInfo(0.5, 2))
#         for i in range(n_densities)
#     ])
#
#     com_moves = dict([
#         (i, 0.15)
#         for i in range(n_densities)
#     ])
#
#     project_info_toml_filepath = Path("..", "project_info_toml_files", "p64_coarse_pert2b3b.toml")
#     project_info = parse_project_info(project_info_toml_filepath)
#
#     # project_info.abs_subproject_dirpath = Path(f"{str(project_info.abs_subproject_dirpath)}{version}")
#     # project_info.subproject_name = f"{project_info.subproject_name}_{version}"
#
#     formatter = BasicProjectDirectoryFormatter()
#     manager = ProjectDirectoryStructureManager(project_info, formatter)
#
#     # make_directories(manager, densities, bisection_moves, com_moves)
#     run_slurm_files(manager, n_densities)


if __name__ == "__main__":
    n_densities = 1
    densities = [0.1]

    bisection_moves = dict([(i, MultibeadPositionMoveInfo(0.9, 3)) for i in range(n_densities)])

    com_moves = dict([(i, 0.0238) for i in range(n_densities)])

    project_info_toml_filepath = Path("..", "project_info_toml_files", "pert2b3b4b", "n48_p960_pert2b3b4b_simulation.toml")
    project_info = parse_project_info(project_info_toml_filepath)
    formatter = BasicProjectDirectoryFormatter()
    manager = ProjectDirectoryStructureManager(project_info, formatter)

    # make_directories(manager, densities, bisection_moves, com_moves)
    # run_slurm_files(manager, n_densities)
