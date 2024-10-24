"""
This module contains functions for specific types of formatting required to create the
directory structure for the project.

Certain functions assume that certain directories have names that are formatted in a
certain way. Rather than forcing the user to handle all these separate cases, this
module contains functions that force them all these cases in the same way.
"""

from abc import ABC
from abc import abstractmethod
from typing import Any


class ProjectDirectoryFormatter(ABC):
    @abstractmethod
    def format_sim_id(self, sim_id: Any) -> str:
        """Format the simulation ID into a string"""


class BasicProjectDirectoryFormatter(ProjectDirectoryFormatter):
    def __init__(self) -> None:
        pass

    def format_sim_id(self, sim_id: int) -> str:
        return f"{sim_id:0>3d}"


# def format_sim_id(sim_id: int) -> str:
#     """
#     Create a specific function to format the simulation ID since it is used in so many
#     different places in this script.
#     """
#     return f"{sim_id:0>3d}"
