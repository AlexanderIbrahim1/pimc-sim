"""
This module contains functions for analyzing the optimal monte carlo step sizes
from the output of a simulation.
"""

from typing import Any

import numpy as np

from pimc_simpy.data import BoxSides
from pimc_simpy.data import HistogramInfo
from pimc_simpy.data import PropertyData
from pimc_simpy.manage import ProjectDirectoryStructureManager

import pimc_simpy.quick_analysis._default_filenames as _default_filenames
from pimc_simpy.quick_analysis._project_data_reader_helper import ProjectDataReaderHelper


class ProjectDataReader:
    def __init__(self, directory_manager: ProjectDirectoryStructureManager) -> None:
        self._helper = ProjectDataReaderHelper(directory_manager)

    def read_project_kinetic_energy(
        self,
        sim_id: Any,
        *,
        normalize_by: float = 1.0,
        filename: str = _default_filenames._KINETIC_ENERGY_FILENAME,
    ) -> PropertyData:
        return self._helper._read_project_property_data(sim_id, filename, normalize_by=normalize_by, dtype=np.float64)

    def read_project_pair_potential_energy(
        self,
        sim_id: Any,
        *,
        normalize_by: float = 1.0,
        filename: str = _default_filenames._PAIR_POTENTIAL_ENERGY_FILENAME,
    ) -> PropertyData:
        return self._helper._read_project_property_data(sim_id, filename, normalize_by=normalize_by, dtype=np.float64)

    def read_project_triplet_potential_energy(
        self,
        sim_id: Any,
        *,
        normalize_by: float = 1.0,
        filename: str = _default_filenames._TRIPLET_POTENTIAL_ENERGY_FILENAME,
    ) -> PropertyData:
        return self._helper._read_project_property_data(sim_id, filename, normalize_by=normalize_by, dtype=np.float64)

    def read_project_quadruplet_potential_energy(
        self,
        sim_id: Any,
        *,
        normalize_by: float = 1.0,
        filename: str = _default_filenames._QUADRUPLET_POTENTIAL_ENERGY_FILENAME,
    ) -> PropertyData:
        return self._helper._read_project_property_data(sim_id, filename, normalize_by=normalize_by, dtype=np.float64)

    def read_project_rms_centroid_distance(
        self,
        sim_id: Any,
        *,
        normalize_by: float = 1.0,
        filename: str = _default_filenames._RMS_CENTROID_DISTANCE_FILENAME,
    ) -> PropertyData:
        return self._helper._read_project_property_data(sim_id, filename, normalize_by=normalize_by, dtype=np.float64)

    def read_project_absolute_centroid_distance(
        self,
        sim_id: Any,
        *,
        normalize_by: float = 1.0,
        filename: str = _default_filenames._ABSOLUTE_CENTROID_DISTANCE_FILENAME,
    ) -> PropertyData:
        return self._helper._read_project_property_data(sim_id, filename, normalize_by=normalize_by, dtype=np.float64)

    def read_project_centre_of_mass_step_size(
        self,
        sim_id: Any,
        *,
        normalize_by: float = 1.0,
        filename: str = _default_filenames._CENTRE_OF_MASS_STEP_SIZE_FILENAME,
    ) -> PropertyData:
        return self._helper._read_project_property_data(sim_id, filename, normalize_by=normalize_by, dtype=np.float64)

    def read_project_bisection_multibead_position_move_acceptance(
        self,
        sim_id: Any,
        *,
        filename: str = _default_filenames._BISECTION_MULTIBEAD_POSITION_MOVE_ACCEPT_FILENAME,
    ) -> tuple[PropertyData, PropertyData]:
        output = self._helper._read_project_property_data_multiple(sim_id, filename, 2, dtypes=(np.int32, np.int32))
        return (output[0], output[1])

    def read_project_single_bead_position_move_acceptance(
        self,
        sim_id: Any,
        *,
        filename: str = _default_filenames._SINGLE_BEAD_POSITION_MOVE_ACCEPT_FILENAME,
    ) -> tuple[PropertyData, PropertyData]:
        output = self._helper._read_project_property_data_multiple(sim_id, filename, 2, dtypes=(np.int32, np.int32))
        return (output[0], output[1])

    def read_project_centre_of_mass_position_move_acceptance(
        self,
        sim_id: Any,
        *,
        filename: str = _default_filenames._CENTRE_OF_MASS_POSITION_MOVE_ACCEPT_FILENAME,
    ) -> tuple[PropertyData, PropertyData]:
        output = self._helper._read_project_property_data_multiple(sim_id, filename, 2, dtypes=(np.int32, np.int32))
        return (output[0], output[1])

    def read_project_bisection_multibead_position_move_info(
        self,
        sim_id: Any,
        *,
        filename: str = _default_filenames._BISECTION_MULTIBEAD_POSITION_MOVE_INFO_FILENAME,
    ) -> tuple[PropertyData, PropertyData]:
        output = self._helper._read_project_property_data_multiple(sim_id, filename, 2, dtypes=(np.float64, np.int32))
        return (output[0], output[1])

    def read_project_timer(
        self,
        sim_id: Any,
        *,
        filename: str = _default_filenames._BISECTION_MULTIBEAD_POSITION_MOVE_INFO_FILENAME,
    ) -> tuple[PropertyData, PropertyData, PropertyData]:
        output = self._helper._read_project_property_data_multiple(sim_id, filename, 3, dtypes=(np.int32, np.int32, np.int32))
        return (output[0], output[1], output[2])

    def read_project_radial_dist_histogram(
        self,
        sim_id: Any,
        *,
        filename: str = _default_filenames._RADIAL_DIST_HISTO_FILENAME,
    ) -> HistogramInfo:
        return self._helper._read_project_histogram(sim_id, filename)

    def read_project_centroid_radial_dist_histogram(
        self,
        sim_id: Any,
        *,
        filename: str = _default_filenames._CENTROID_RADIAL_DIST_HISTO_FILENAME,
    ) -> HistogramInfo:
        return self._helper._read_project_histogram(sim_id, filename)

    def read_project_box_sides(
        self,
        sim_id: Any,
        *,
        filename: str = _default_filenames._CENTROID_RADIAL_DIST_HISTO_FILENAME,
    ) -> BoxSides:
        return self._helper._read_project_box_sides(sim_id, filename)
