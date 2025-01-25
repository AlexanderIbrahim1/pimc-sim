"""
This script contains code to run identical simulations, but each with a different
worldline file as a starting seed.
"""

import subprocess
import shutil
from pathlib import Path
from typing import Any

from pimc_simpy.data import MultibeadPositionMoveInfo
from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager
from pimc_simpy.manage import parse_project_info

from shared_file_contents import get_toml_file_contents
from shared_file_contents import get_slurm_file_contents


def copy_worldline_seed(
    worldline_seed_dirpath: Path,
    destination_dirpath: Path,
    i_worldline: int,
) -> None:
    worldline_filename = f"worldline{i_worldline:0>5d}.dat"
    source_filepath = worldline_seed_dirpath / worldline_filename

    shutil.copy(source_filepath, destination_dirpath)


def copy_continue_file(
    continue_seed_dirpath: Path,
    destination_dirpath: Path,
) -> None:
    source_filepath = continue_seed_dirpath / "continue.toml"
    shutil.copy(source_filepath, destination_dirpath)


def rename_worldline_seed(
    simulation_dirpath: Path,
    i_worldline_current: int,
    i_worldline_new: int,
) -> None:
    current_worldline_filename = f"worldline{i_worldline_current:0>5d}.dat"
    new_worldline_filename = f"worldline{i_worldline_new:0>5d}.dat"

    current_filepath = simulation_dirpath / current_worldline_filename
    new_filepath = simulation_dirpath / new_worldline_filename

    current_filepath.rename(new_filepath)


def make_directories(
    manager: ProjectDirectoryStructureManager,
    continue_seed_dirpath: Path,
    worldline_seed_dirpath: Path,
    worldline_index_pairs: list[tuple[int, int]],
    density: float,
    bisection_move: MultibeadPositionMoveInfo,
    com_step_size: float,
) -> None:
    toml_info_map: dict[str, Any] = {}
    toml_info_map["abs_repo_dirpath"] = manager.info.abs_external_dirpath
    toml_info_map["cell_dimensions"] = (5, 3, 3)
    toml_info_map["seed"] = '"RANDOM"'
    toml_info_map["last_block_index"] = 160
    toml_info_map["n_equilibrium_blocks"] = 10
    toml_info_map["n_passes"] = 1
    toml_info_map["n_timeslices"] = 64
    toml_info_map["writer_batch_size"] = 1
    toml_info_map["save_worldlines"] = "true"
    toml_info_map["n_save_worldlines_every"] = 10
    toml_info_map["freeze_mc_steps"] = "true"

    toml_info_map["density"] = density

    # set the converged monte carlo step sizes
    toml_info_map["centre_of_mass_step_size"] = com_step_size
    toml_info_map["bisection_level"] = bisection_move.lower_level
    toml_info_map["bisection_ratio"] = bisection_move.upper_level_fraction

    slurm_info_map: dict[str, Any] = {}
    slurm_info_map["executable"] = "perturbative2b3b4b"
    slurm_info_map["abs_executable_dirpath"] = manager.info.abs_executable_dirpath
    slurm_info_map["memory_gb"] = 4
    slurm_info_map["time"] = "2-00:00:00"

    manager.mkdir_subproject_dirpaths()

    for sim_id, (i_worldline_old, i_worldline_new) in enumerate(worldline_index_pairs):
        # create the locations for the simulation and the output
        manager.mkdir_job_and_output_dirpaths(sim_id)

        toml_info_map["abs_output_dirpath"] = manager.get_abs_simulations_job_output_dirpath(sim_id)

        # copy the worldline seed to the simulation directory, and rename it to something
        # that the simulation is able to seed from
        copy_worldline_seed(worldline_seed_dirpath, toml_info_map["abs_output_dirpath"], i_worldline_old)
        rename_worldline_seed(toml_info_map["abs_output_dirpath"], i_worldline_old, i_worldline_new)
        copy_continue_file(continue_seed_dirpath, toml_info_map["abs_output_dirpath"])

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


def run_slurm_files(manager: ProjectDirectoryStructureManager, n_jobs: int) -> None:
    for sim_id in range(1, n_jobs):
        abs_slurm_filepath = manager.get_slurm_bashfile_filepath(sim_id)

        cmd = ["sbatch", str(abs_slurm_filepath)]
        subprocess.run(cmd, check=True)


if __name__ == "__main__":
    density = 0.1
    bisection_move = MultibeadPositionMoveInfo(0.0, 1)
    com_step_size = 2.66666804e-02
    worldline_index_pairs = [(i, 0) for i in range(200, 1200)]

    project_info_toml_filepath = Path("..", "project_info_toml_files", "p64_coarse", "pert2b3b4b_dens_030.toml")
    project_info = parse_project_info(project_info_toml_filepath)
    formatter = BasicProjectDirectoryFormatter()
    manager = ProjectDirectoryStructureManager(project_info, formatter)

    continue_seed_dirpath = Path("..", "playground", "run_identical_seeded_simulations")
    worldline_seed_dirpath = Path("/home/a68ibrah/projects/def-pnroy/a68ibrah/pimc_simulations/simulations/twothreefour_body/p64_coarse/worldline_seeds/dens_030/simulations/job_000/output")

    # make_directories(
    #     manager,
    #     continue_seed_dirpath,
    #     worldline_seed_dirpath,
    #     worldline_index_pairs,
    #     density,
    #     bisection_move,
    #     com_step_size,
    # )

    n_jobs = len(worldline_index_pairs)
    run_slurm_files(manager, n_jobs)
