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
    all_n_timeslices: list[int] = [64, 80, 96, 128, 192]
    all_versions: list[int] = list(range(10))

    most_recent_block_index = 1999
    job_ids = list(range(21))

    for n_timeslices in all_n_timeslices:
        for version in all_versions:
            print(f"CHECKING: ({n_timeslices:0>3d}, {version})")
            project_info_toml_dirpath = Path("..", "project_info_toml_files", "equilibrium_density_files")
            project_info_toml_filename = f"pert2b3b_eq_dens_tmpl.toml"
            project_info_toml_filepath = project_info_toml_dirpath / project_info_toml_filename
            project_info = parse_project_info(project_info_toml_filepath)
            project_info.abs_subproject_dirpath = project_info.abs_subproject_dirpath / f"p{n_timeslices:0>3d}" / f"version{version}"
            project_info.subproject_name = f"eq_dens_p{n_timeslices:0>3d}_version{version}"

            formatter = BasicProjectDirectoryFormatter()
            manager = ProjectDirectoryStructureManager(project_info, formatter)

            for job_id in job_ids:
                try:
                    check_job(job_id, manager, most_recent_block_index)
                except JobSuccessException as exc:
                    print(f"({n_timeslices:0>3d}, {version}, {job_id:0>3d}) : {exc}")


if __name__ == "__main__":
    main()
