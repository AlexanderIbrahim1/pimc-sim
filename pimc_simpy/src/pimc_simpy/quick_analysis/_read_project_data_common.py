"""
This module contains functions for analyzing the optimal monte carlo step sizes
from the output of a simulation.
"""

import numpy as np

from pimc_simpy.manage import ProjectInfo

from pimc_simpy.data import BoxSides
from pimc_simpy.data import HistogramInfo
from pimc_simpy.data import PropertyData
from pimc_simpy.data import PropertyStatistics
from pimc_simpy.data import get_property_statistics
from pimc_simpy.data import read_histogram
from pimc_simpy.data import read_property_data
from pimc_simpy.data import read_property_data_multiple
from pimc_simpy.data import between_epochs
from pimc_simpy.data import last_n_epochs

import pimc_simpy.quick_analysis._default_filenames as _default_filenames
from pimc_simpy.quick_analysis._read_project_data_helpers import _read_project_box_sides
from pimc_simpy.quick_analysis._read_project_data_helpers import _read_project_histogram
from pimc_simpy.quick_analysis._read_project_data_helpers import _read_project_property_data
from pimc_simpy.quick_analysis._read_project_data_helpers import _read_project_property_data_multiple


def read_project_kinetic_energy(
    info: ProjectInfo,
    sim_id: int,
    *,
    normalize_by: float = 1.0,
    filename: str = _default_filenames._KINETIC_ENERGY_FILENAME,
) -> PropertyData:
    return _read_project_property_data(info, sim_id, filename, normalize_by=normalize_by, dtype=np.float64)


def read_project_pair_potential_energy(
    info: ProjectInfo,
    sim_id: int,
    *,
    normalize_by: float = 1.0,
    filename: str = _default_filenames._PAIR_POTENTIAL_ENERGY_FILENAME,
) -> PropertyData:
    return _read_project_property_data(info, sim_id, filename, normalize_by=normalize_by, dtype=np.float64)


def read_project_triplet_potential_energy(
    info: ProjectInfo,
    sim_id: int,
    *,
    normalize_by: float = 1.0,
    filename: str = _default_filenames._TRIPLET_POTENTIAL_ENERGY_FILENAME,
) -> PropertyData:
    return _read_project_property_data(info, sim_id, filename, normalize_by=normalize_by, dtype=np.float64)


def read_project_quadruplet_potential_energy(
    info: ProjectInfo,
    sim_id: int,
    *,
    normalize_by: float = 1.0,
    filename: str = _default_filenames._QUADRUPLET_POTENTIAL_ENERGY_FILENAME,
) -> PropertyData:
    return _read_project_property_data(info, sim_id, filename, normalize_by=normalize_by, dtype=np.float64)


def read_project_rms_centroid_distance(
    info: ProjectInfo,
    sim_id: int,
    *,
    normalize_by: float = 1.0,
    filename: str = _default_filenames._RMS_CENTROID_DISTANCE_FILENAME,
) -> PropertyData:
    return _read_project_property_data(info, sim_id, filename, normalize_by=normalize_by, dtype=np.float64)


def read_project_absolute_centroid_distance(
    info: ProjectInfo,
    sim_id: int,
    *,
    normalize_by: float = 1.0,
    filename: str = _default_filenames._ABSOLUTE_CENTROID_DISTANCE_FILENAME,
) -> PropertyData:
    return _read_project_property_data(info, sim_id, filename, normalize_by=normalize_by, dtype=np.float64)


def read_project_centre_of_mass_step_size(
    info: ProjectInfo,
    sim_id: int,
    *,
    normalize_by: float = 1.0,
    filename: str = _default_filenames._CENTRE_OF_MASS_STEP_SIZE_FILENAME,
) -> PropertyData:
    return _read_project_property_data(info, sim_id, filename, normalize_by=normalize_by, dtype=np.float64)


def read_project_bisection_multibead_position_move_acceptance(
    info: ProjectInfo,
    sim_id: int,
    *,
    filename: str = _default_filenames._BISECTION_MULTIBEAD_POSITION_MOVE_ACCEPT_FILENAME,
) -> tuple[PropertyData, PropertyData]:
    output = _read_project_property_data_multiple(info, sim_id, filename, 2, dtypes=(np.int32, np.int32))
    return (output[0], output[1])


def read_project_single_bead_position_move_acceptance(
    info: ProjectInfo,
    sim_id: int,
    *,
    filename: str = _default_filenames._SINGLE_BEAD_POSITION_MOVE_ACCEPT_FILENAME,
) -> tuple[PropertyData, PropertyData]:
    output = _read_project_property_data_multiple(info, sim_id, filename, 2, dtypes=(np.int32, np.int32))
    return (output[0], output[1])


def read_project_centre_of_mass_position_move_acceptance(
    info: ProjectInfo,
    sim_id: int,
    *,
    filename: str = _default_filenames._CENTRE_OF_MASS_POSITION_MOVE_ACCEPT_FILENAME,
) -> tuple[PropertyData, PropertyData]:
    output = _read_project_property_data_multiple(info, sim_id, filename, 2, dtypes=(np.int32, np.int32))
    return (output[0], output[1])


def read_project_bisection_multibead_position_move_info(
    info: ProjectInfo,
    sim_id: int,
    *,
    filename: str = _default_filenames._BISECTION_MULTIBEAD_POSITION_MOVE_INFO_FILENAME,
) -> tuple[PropertyData, PropertyData]:
    output = _read_project_property_data_multiple(info, sim_id, filename, 2, dtypes=(np.float64, np.int32))
    return (output[0], output[1])


def read_project_timer(
    info: ProjectInfo,
    sim_id: int,
    *,
    filename: str = _default_filenames._BISECTION_MULTIBEAD_POSITION_MOVE_INFO_FILENAME,
) -> tuple[PropertyData, PropertyData, PropertyData]:
    output = _read_project_property_data_multiple(info, sim_id, filename, 3, dtypes=(np.int32, np.int32, np.int32))
    return (output[0], output[1], output[2])


def read_project_radial_dist_histogram(
    info: ProjectInfo,
    sim_id: int,
    *,
    filename: str = _default_filenames._RADIAL_DIST_HISTO_FILENAME,
) -> HistogramInfo:
    return _read_project_histogram(info, sim_id, filename)


def read_project_centroid_radial_dist_histogram(
    info: ProjectInfo,
    sim_id: int,
    *,
    filename: str = _default_filenames._CENTROID_RADIAL_DIST_HISTO_FILENAME,
) -> HistogramInfo:
    return _read_project_histogram(info, sim_id, filename)


def read_project_box_sides(
    info: ProjectInfo,
    sim_id: int,
    *,
    filename: str = _default_filenames._CENTROID_RADIAL_DIST_HISTO_FILENAME,
) -> BoxSides:
    return _read_project_box_sides(info, sim_id, filename)


# def collect_centre_of_mass_step_size(
#     info: ProjectInfo, sim_id: int, n_last: Optional[int] = None, *, com_step_size_filename: Optional[str] = None
# ) -> PropertyData:
#     abs_job_output_dirpath = get_abs_simulations_job_output_dirpath(info, sim_id)
#
#     if not com_step_size_filename:
#         com_step_size_filename = CENTRE_OF_MASS_STEP_SIZE_FILENAME_
#
#     com_step_size_filepath = abs_job_output_dirpath / com_step_size_filename
#
#     com_data = read_property_data(com_step_size_filepath, dtype=np.float64)
#
#     if not n_last:
#         com_data = last_n_epochs(n_last, com_data)
#
#     return com_data
#
#
# def collect_centre_of_mass_step_size_statistics(
#     info: ProjectInfo, sim_id: int, n_last: Optional[int] = None, *, com_step_size_filename: Optional[str] = None
# ) -> PropertyStatistics:
#     com_data = collect_centre_of_mass_step_size(info, sim_id, n_last=n_last, com_step_size_filename=com_step_size_filename)
#
#     return get_property_statistics(com_data)
#
#
# def collect_all_com_step_size_info() -> None:
#     n_densities: int = 31
#     info = ProjectInfo()
#
#     com_step_sizes = np.empty(n_densities, dtype=np.float64)
#
#     out_filepath = Path("ideal_mc_steps", "ideal_com_step_sizes.dat")
#
#     for sim_id in range(n_densities):
#         abs_job_output_dirpath = get_abs_job_output_dirpath(info, sim_id)
#         com_step_sizes[sim_id] = collect_com_step_size_info(abs_job_output_dirpath, 100)
#
#     np.savetxt(out_filepath, com_step_sizes)
#
#
# def collect_bisection_step_size_info(abs_job_output_dirpath: Path, n_last_to_average: int) -> tuple[float, int]:
#     filepath = abs_job_output_dirpath / BISECTION_MOVE_INFO_FILENAME
#
#     upper_fractions, lower_levels = read_property_data_multiple(filepath, n_data=2, dtypes=(np.float64, np.int32))
#
#     i_epoch_right = upper_fractions.epochs[-1]
#     i_epoch_left = i_epoch_right - n_last_to_average
#
#     upper_fractions = between_epochs(i_epoch_left, i_epoch_right, upper_fractions)
#     lower_levels = between_epochs(i_epoch_left, i_epoch_right, lower_levels)
#
#     average_upper_fraction = np.average(upper_fractions.values).astype(float)
#     average_lower_level = np.average(lower_levels.values).astype(int)
#
#     return average_upper_fraction, average_lower_level
#
#
# def collect_all_bisection_step_size_info() -> None:
#     n_densities: int = 31
#     info = ProjectInfo()
#
#     out_filepath = Path("ideal_mc_steps", "ideal_bisection_step_sizes.dat")
#
#     with open(out_filepath, "w") as fout:
#         for sim_id in range(n_densities):
#             abs_job_output_dirpath = get_abs_job_output_dirpath(info, sim_id)
#             upper_fraction, lower_level = collect_bisection_step_size_info(abs_job_output_dirpath, 100)
#             fout.write(f"{upper_fraction: 12.8f}   {lower_level}\n")
#
#
# if __name__ == "__main__":
#     collect_all_bisection_step_size_info()
