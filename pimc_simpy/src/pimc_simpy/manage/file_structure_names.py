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
   |- output/    |- output/    |- output/       |- subproject_000.sh      |- slurm-000-12345678.out
   |- sim.toml   |- sim.toml   |- sim.toml      |- subproject_001.sh      |- slurm-001-10931092.out
                                                |- subproject_002.sh      |- slurm-002-09301923.out
"""

from pathlib import Path
from typing import Any

from pimc_simpy.manage.formatting import ProjectDirectoryFormatter
from pimc_simpy.manage.project_info import ProjectInfo


DEFAULT_TOML_FILENAME = "sim.toml"  # okay to have it be the same for each simulation; makes commands simpler


class ProjectDirectoryStructureManager:
    def __init__(
        self, info: ProjectInfo, formatter: ProjectDirectoryFormatter, toml_filename: str = DEFAULT_TOML_FILENAME
    ) -> None:
        self._toml_filename = toml_filename
        self._info = info
        self._formatter = formatter

    @property
    def info(self) -> ProjectInfo:
        return self._info

    def get_abs_simulations_job_output_dirpath(self, sim_id: Any) -> Path:
        return self._get_abs_simulations_job_dirpath(sim_id) / "output"

    def get_abs_slurm_output_filename(self, sim_id: Any) -> str:
        abs_slurm_output_dirpath = self._get_abs_slurm_output_dirpath()
        slurm_output_filename = f"slurm-{self._formatter.format_sim_id(sim_id)}-%j.out"
        return f"{str(abs_slurm_output_dirpath)}/{slurm_output_filename}"

    def get_slurm_bashfile_filepath(self, sim_id: Any) -> Path:
        slurm_filename = self._get_slurm_bash_filename(sim_id)
        return self._get_abs_slurm_bashfiles_dirpath() / slurm_filename

    def get_toml_filepath(self, sim_id: Any) -> Path:
        return self._get_abs_simulations_job_dirpath(sim_id) / self._toml_filename

    def mkdir_subproject_dirpaths(self) -> None:
        self._get_abs_simulations_dirpath().mkdir(exist_ok=True)
        self._get_abs_slurm_dirpath().mkdir(exist_ok=True)
        self._get_abs_slurm_bashfiles_dirpath().mkdir(exist_ok=True)
        self._get_abs_slurm_output_dirpath().mkdir(exist_ok=True)

    def mkdir_job_and_output_dirpaths(self, sim_id: Any) -> None:
        self._get_abs_simulations_job_dirpath(sim_id).mkdir(exist_ok=True)
        self.get_abs_simulations_job_output_dirpath(sim_id).mkdir(exist_ok=True)

    def _get_abs_simulations_job_dirpath(self, sim_id: Any) -> Path:
        job_dirname = f"job_{self._formatter.format_sim_id(sim_id)}"
        return self._get_abs_simulations_dirpath() / job_dirname

    def _get_abs_slurm_bashfiles_dirpath(self) -> Path:
        return self._get_abs_slurm_dirpath() / "slurm_bashfiles"

    def _get_abs_slurm_output_dirpath(self) -> Path:
        return self._get_abs_slurm_dirpath() / "slurm_output"

    def _get_abs_simulations_dirpath(self) -> Path:
        return self._info.abs_subproject_dirpath / "simulations"

    def _get_abs_slurm_dirpath(self) -> Path:
        return self._info.abs_subproject_dirpath / "slurm"

    def _get_slurm_bash_filename(self, sim_id: Any) -> str:
        return f"{self._info.subproject_name}_{self._formatter.format_sim_id(sim_id)}.sh"
