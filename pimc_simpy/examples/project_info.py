"""
This file contains the ProjectInfo instance that determines the "root" directory
of the collection of jobs for the particular project.

This file will be accessed by several other files that need to know about the
directory structure in order to extract information from those job files.
"""

from pathlib import Path

from pimc_simpy.manage import SimulationProjectInfo


# @dataclass
# class SimulationProjectInfo:
#     abs_a68home = Path("/home/a68ibrah/projects/def-pnroy/a68ibrah")
#     abs_repo_dirpath = abs_a68home / "pimc_simulations" / "pimc-sim"
#     abs_executable_filepath = abs_repo_dirpath / "build" / "dev-highperf" / "source" / "pimc-sim"
#     abs_project_dirpath = abs_a68home / "pimc_simulations" / "simulations" / "twothreefour_body"
#     abs_subproject_dirpath = abs_project_dirpath / "mcmc_param_search" / "p64_coarse"
#     subproject_name = "p64_coarse"


# class ProjectInfo(SimulationProjectInfo):
#     def __init__(self) -> None:
#         self.abs_a68home = Path("/home/a68ibrah/research/simulations")
#         self.abs_project_dirpath = self.abs_a68home / "pimc_simulations" / "simulations" / "twothreefour_body"
# 
#     @property
#     def abs_repo_dirpath(self) -> Path:
#         return self.abs_a68home / "pimc-sim"
# 
#     @property
#     def abs_executable_filepath(self) -> Path:
#         return self.abs_repo_dirpath / "build" / "dev-highperf" / "source" / "pimc-sim"
# 
#     @property
#     def abs_subproject_dirpath(self) -> Path:
#         return self.abs_project_dirpath / "mcmc_param_search" / "p64_coarse"
# 
#     @property
#     def subproject_name(self) -> str:
#         return "p64_coarse"


class ProjectInfo(SimulationProjectInfo):
    def __init__(self) -> None:
        self.abs_a68home = Path("/home/a68ibrah/projects/def-pnroy/a68ibrah")
        self.abs_project_dirpath = self.abs_a68home / "pimc_simulations" / "simulations" / "twothreefour_body"

    @property
    def abs_repo_dirpath(self) -> Path:
        return self.abs_a68home / "pimc_simulations" / "pimc-sim"

    @property
    def abs_executable_filepath(self) -> Path:
        return self.abs_repo_dirpath / "build" / "dev-highperf" / "source" / "pimc-sim"

    @property
    def abs_subproject_dirpath(self) -> Path:
        return self.abs_project_dirpath / "mcmc_param_search" / "p64_coarse"

    @property
    def subproject_name(self) -> str:
        return "p64_coarse"
