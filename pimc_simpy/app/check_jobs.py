"""
This script contains code for checking if a job has run properly.
"""

import tomllib
from pathlib import Path

from pimc_simpy.manage import parse_project_info
from pimc_simpy.manage import BasicProjectDirectoryFormatter
from pimc_simpy.manage import ProjectDirectoryStructureManager


class JobSuccessException(Exception):
    pass


def get_project_manager(project_info_toml_filepath: Path) -> ProjectDirectoryStructureManager:
    info = parse_project_info(project_info_toml_filepath)
    formatter = BasicProjectDirectoryFormatter()
    manager = ProjectDirectoryStructureManager(info, formatter)

    return manager


def check_job(job_id: int, manager: ProjectDirectoryStructureManager, most_recent_block_index: int) -> None:
    output_dirpath = manager.get_abs_simulations_job_output_dirpath(job_id)

    prefix = f"output dirpath for job {job_id:0>3d}"

    if not output_dirpath.exists():
        raise JobSuccessException("f{prefix} does not exist")

    if not any(output_dirpath.iterdir()):
        raise JobSuccessException(f"{prefix} is empty")

    continue_filepath: Path = output_dirpath / "continue.toml"
    if not continue_filepath.exists():
        raise JobSuccessException(f"{prefix} has no 'continue.toml' file")

    with open(continue_filepath, "rb") as fin:
        toml_data = tomllib.load(fin)
        toml_most_recent_block_index: int = toml_data.get("most_recent_block_index")

        if toml_most_recent_block_index != most_recent_block_index:
            raise JobSuccessException(f"{prefix} did not reach the required 'most_recent_block_index'")


def main() -> None:
    most_recent_block_index = 1000
    version = 0
    job_ids = list(range(31))

    project_info_toml_filepath = Path("..", "project_info_toml_files", "p960_coarse_pert2b3b_tmpl.toml")
    info = parse_project_info(project_info_toml_filepath)
    info.abs_subproject_dirpath = Path(f"{str(info.abs_subproject_dirpath)}{version}")
    info.subproject_name = f"{info.subproject_name}_{version}"

    formatter = BasicProjectDirectoryFormatter()
    manager = ProjectDirectoryStructureManager(info, formatter)

    for job_id in job_ids:
        try:
            check_job(job_id, manager, most_recent_block_index)
        except JobSuccessException as exc:
            print(f"{job_id:0>3d} : {exc}")


if __name__ == "__main__":
    main()
