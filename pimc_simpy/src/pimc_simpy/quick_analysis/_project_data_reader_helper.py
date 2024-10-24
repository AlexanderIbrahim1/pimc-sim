"""
This module contains the underlying functions for reading information from the
output files of a simulation of a project described by a ProjectInfo object.
"""

from typing import Any
from typing import Optional
from typing import Type

from pimc_simpy.manage import ProjectDirectoryStructureManager

from pimc_simpy.data import BoxSides
from pimc_simpy.data import HistogramInfo
from pimc_simpy.data import PropertyData
from pimc_simpy.data import read_box_sides
from pimc_simpy.data import read_histogram
from pimc_simpy.data import read_property_data
from pimc_simpy.data import read_property_data_multiple


class ProjectDataReaderHelper:
    def __init__(self, directory_manager: ProjectDirectoryStructureManager) -> None:
        self._directory_manager = directory_manager

    def _read_project_property_data(
        self,
        sim_id: Any,
        filename: str,
        *,
        normalize_by: float,
        dtype: Type,
    ) -> PropertyData:
        abs_job_output_dirpath = self._directory_manager.get_abs_simulations_job_output_dirpath(sim_id)
        filepath = abs_job_output_dirpath / filename

        return read_property_data(filepath, normalize_by=normalize_by, dtype=dtype)

    def _read_project_property_data_multiple(
        self,
        sim_id: Any,
        filename: str,
        n_data: int,
        *,
        normalize_by: Optional[tuple[float, ...]] = None,
        dtypes: Optional[tuple[Type, ...]] = None,
    ) -> list[PropertyData]:
        abs_job_output_dirpath = self._directory_manager.get_abs_simulations_job_output_dirpath(sim_id)
        filepath = abs_job_output_dirpath / filename

        return read_property_data_multiple(filepath, n_data, normalize_by=normalize_by, dtypes=dtypes)

    def _read_project_histogram(
        self,
        sim_id: Any,
        filename: str,
    ) -> HistogramInfo:
        abs_job_output_dirpath = self._directory_manager.get_abs_simulations_job_output_dirpath(sim_id)
        filepath = abs_job_output_dirpath / filename

        return read_histogram(filepath)

    def _read_project_box_sides(
        self,
        sim_id: Any,
        filename: str,
    ) -> BoxSides:
        abs_job_output_dirpath = self._directory_manager.get_abs_simulations_job_output_dirpath(sim_id)
        filepath = abs_job_output_dirpath / filename

        return read_box_sides(filepath)
