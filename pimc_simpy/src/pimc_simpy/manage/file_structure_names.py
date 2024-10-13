"""
This module contains functions that make it easier to create the directories used in
the file structure of the project.

The tree should look something like this:

                                           subproject
                                               |
                       +-----------------------+----------------------+
                       |                                              |
                  simulations                                       slurm
                       |                                              |
      +-------------+--+----------+                    +--------------+--------+
      |             |             |                    |                       |
   job_000       job_001       job_002          slurm_bashfiles           slurm_output
   |             |             |                |                         |
   |- output/    |- output/    |- output/       |- project_000.sh         |- slurm-000-12345678.out
   |- sim.toml   |- sim.toml   |- sim.toml      |- project_001.sh         |- slurm-001-10931092.out
                                                |- project_002.sh         |- slurm-002-09301923.out
"""

from pathlib import Path

from pimc_simpy.manage.formatting import format_sim_id
from pimc_simpy.manage.simulation_project_info import SimulationProjectInfo


TOML_FILENAME = "sim.toml"  # okay to have it be the same for each simulation; makes commands simpler


def _get_slurm_bash_filename(info: SimulationProjectInfo, sim_id: int) -> str:
    return f"{info.subproject_name}_{format_sim_id(sim_id)}.sh"


def _get_abs_simulations_dirpath(info: SimulationProjectInfo) -> Path:
    return info.abs_subproject_dirpath / "simulations"


def _get_abs_slurm_dirpath(info: SimulationProjectInfo) -> Path:
    return info.abs_subproject_dirpath / "slurm"


def _get_abs_slurm_bashfiles_dirpath(info: SimulationProjectInfo) -> Path:
    return _get_abs_slurm_dirpath(info) / "slurm_bashfiles"


def _get_abs_slurm_output_dirpath(info: SimulationProjectInfo) -> Path:
    return _get_abs_slurm_dirpath(info) / "slurm_output"


def _get_abs_simulations_job_dirpath(info: SimulationProjectInfo, sim_id: int) -> Path:
    return _get_abs_simulations_dirpath(info) / f"job_{format_sim_id(sim_id)}"


def get_abs_simulations_job_output_dirpath(info: SimulationProjectInfo, sim_id: int) -> Path:
    return _get_abs_simulations_job_dirpath(info, sim_id) / "output"


def get_abs_slurm_output_filename(info: SimulationProjectInfo, sim_id: int) -> str:
    abs_slurm_output_dirpath = _get_abs_slurm_output_dirpath(info)
    return f"{str(abs_slurm_output_dirpath)}/slurm-{format_sim_id(sim_id)}-%j.out"


def get_slurm_bashfile_filepath(info: SimulationProjectInfo, sim_id: int) -> Path:
    slurm_filename = _get_slurm_bash_filename(info, sim_id)
    return _get_abs_slurm_bashfiles_dirpath(info) / slurm_filename


def get_toml_filepath(info: SimulationProjectInfo, sim_id: int) -> Path:
    return _get_abs_simulations_job_dirpath(info, sim_id) / TOML_FILENAME


def mkdir_subproject_dirpaths(info: SimulationProjectInfo) -> None:
    _get_abs_simulations_dirpath(info).mkdir(exist_ok=True)
    _get_abs_slurm_dirpath(info).mkdir(exist_ok=True)
    _get_abs_slurm_bashfiles_dirpath(info).mkdir(exist_ok=True)
    _get_abs_slurm_output_dirpath(info).mkdir(exist_ok=True)


def mkdir_job_and_output_dirpaths(info: SimulationProjectInfo, sim_id: int) -> None:
    _get_abs_simulations_job_dirpath(info, sim_id).mkdir(exist_ok=True)
    get_abs_simulations_job_output_dirpath(info, sim_id).mkdir(exist_ok=True)
