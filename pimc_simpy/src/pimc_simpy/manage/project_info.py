"""
This module contains the ProjectInfo class, which holds various paths and file names
relevant to managing a collection of simulations.

The ProjectInfo object is meant to contains paths and names that make it convenient
to manage projects from the Python side; not the C++ side. It is the Python side that
uses the ProjectInfo object to create the bash scripts for the simulations.

This means the ProjectInfo object might contain information that isn't *directly*
useful for creating the files that run the simulation, but can be transformed into
information that *is* useful. Meanwhile, the bash scripts that the C++ files use,
will have very specific *direct* information (like paths that are only relevant for
that specific simulation, and not others in the same subproject).
"""

from dataclasses import dataclass
from pathlib import Path
from typing import Any
from typing import BinaryIO
import tomllib


@dataclass
class ProjectInfo:
    abs_external_dirpath: Path
    abs_executable_dirpath: Path
    abs_subproject_dirpath: Path
    subproject_name: str


def parse_project_info(toml_stream: BinaryIO) -> ProjectInfo:
    toml_config = tomllib.load(toml_stream)

    if TOML_ABS_HOME_KEY_ in toml_config:
        return parse_project_info_using_rel_paths_(toml_config)
    else:
        return parse_project_info_using_abs_paths_(toml_config)


# --- implementation details below

TOML_ABS_HOME_KEY_: str = "abs_home"
TOML_SUBPROJECT_NAME_KEY_: str = "subproject_name"

TOML_REL_EXTERNAL_DIRPATH_KEY_: str = "rel_external_dirpath"
TOML_REL_EXECUTABLE_DIRPATH_KEY_: str = "rel_executable_dirpath"
TOML_REL_SUBPROJECT_DIRPATH_KEY_: str = "rel_subproject_dirpath"

TOML_ABS_EXTERNAL_DIRPATH_KEY_: str = "abs_external_dirpath"
TOML_ABS_EXECUTABLE_DIRPATH_KEY_: str = "abs_executable_dirpath"
TOML_ABS_SUBPROJECT_DIRPATH_KEY_: str = "abs_subproject_dirpath"


def parse_project_info_using_rel_paths_(toml_config: dict[str, Any]) -> ProjectInfo:
    abs_home = Path(toml_config[TOML_ABS_HOME_KEY_])

    abs_external_dirpath: Path = abs_home / Path(toml_config[TOML_REL_EXTERNAL_DIRPATH_KEY_])
    abs_executable_filepath: Path = abs_home / Path(toml_config[TOML_REL_EXECUTABLE_DIRPATH_KEY_])
    abs_subproject_dirpath: Path = abs_home / Path(toml_config[TOML_REL_SUBPROJECT_DIRPATH_KEY_])
    subproject_name: str = toml_config[TOML_SUBPROJECT_NAME_KEY_]

    return ProjectInfo(abs_external_dirpath, abs_executable_filepath, abs_subproject_dirpath, subproject_name)


def parse_project_info_using_abs_paths_(toml_config: dict[str, Any]) -> ProjectInfo:
    abs_external_dirpath: Path = Path(toml_config[TOML_ABS_EXTERNAL_DIRPATH_KEY_])
    abs_executable_filepath: Path = Path(toml_config[TOML_ABS_EXECUTABLE_DIRPATH_KEY_])
    abs_subproject_dirpath: Path = Path(toml_config[TOML_ABS_SUBPROJECT_DIRPATH_KEY_])
    subproject_name: str = toml_config[TOML_SUBPROJECT_NAME_KEY_]

    return ProjectInfo(abs_external_dirpath, abs_executable_filepath, abs_subproject_dirpath, subproject_name)
