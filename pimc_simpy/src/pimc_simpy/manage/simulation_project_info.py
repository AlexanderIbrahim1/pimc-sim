"""
This module contains the SimulationProjectInfo abstract base class. The user can create
a class that implements its interface to gain access to functions that make it easier to
manage the directory structure of projects with several simulations.
"""

from abc import ABC
from abc import abstractmethod
from pathlib import Path


class SimulationProjectInfo(ABC):
    @property
    @abstractmethod
    def abs_repo_dirpath(self) -> Path:
        pass

    @property
    @abstractmethod
    def abs_executable_filepath(self) -> Path:
        pass

    @property
    @abstractmethod
    def abs_subproject_dirpath(self) -> Path:
        pass

    @property
    @abstractmethod
    def subproject_name(self) -> str:
        pass
