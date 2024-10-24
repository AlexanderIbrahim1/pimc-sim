"""
This module contains components to read and write the converged monte carlo
step sizes from simulations from a project described by a ProjectInfo object.
"""

from pathlib import Path
from collections.abc import Sequence
from typing import Union

from pimc_simpy.data import MultibeadPositionMoveInfo
from pimc_simpy.data import PropertyData
from pimc_simpy.data import last_n_epochs
from pimc_simpy.data import between_epochs
from pimc_simpy.data import get_property_statistics

from pimc_simpy.quick_analysis._read_project_data_common import ProjectDataReader


def _get_mean_between_epochs(data: PropertyData, between: tuple[int, int]) -> float:
    data = between_epochs(between[0], between[1], data)
    stats = get_property_statistics(data)
    return stats.mean


def _get_mean_last_n_epochs(data: PropertyData, last_n: int) -> float:
    data = last_n_epochs(last_n, data)
    stats = get_property_statistics(data)
    return stats.mean


def write_converged_bisection_multibead_position_move_info_between(
    reader: ProjectDataReader,
    output_filepath: Path,
    sim_ids: Sequence[int],
    between: tuple[int, int],
) -> None:
    with open(output_filepath, "w") as fout:
        for sim_id in sim_ids:
            upper_fractions, lower_levels = reader.read_project_bisection_multibead_position_move_info(sim_id)

            upper_fraction_mean = _get_mean_between_epochs(upper_fractions, between)
            lower_level_mean = round(_get_mean_between_epochs(lower_levels, between))

            fout.write(f"{sim_id:>2d}  {upper_fraction_mean: 12.8f}  {lower_level_mean:>2d}\n")


def write_converged_bisection_multibead_position_move_info_last(
    reader: ProjectDataReader,
    output_filepath: Path,
    sim_ids: Sequence[int],
    n_last: int,
) -> None:
    with open(output_filepath, "w") as fout:
        for sim_id in sim_ids:
            upper_fractions, lower_levels = reader.read_project_bisection_multibead_position_move_info(sim_id)

            upper_fraction_mean = _get_mean_last_n_epochs(upper_fractions, n_last)
            lower_level_mean = round(_get_mean_last_n_epochs(lower_levels, n_last))

            fout.write(f"{sim_id:>2d}  {upper_fraction_mean: 12.8f}  {lower_level_mean:>2d}\n")


def write_converged_centre_of_mass_step_size_between(
    reader: ProjectDataReader,
    output_filepath: Path,
    sim_ids: Sequence[int],
    between: tuple[int, int],
) -> None:
    with open(output_filepath, "w") as fout:
        for sim_id in sim_ids:
            com_step_sizes = reader.read_project_centre_of_mass_step_size(sim_id)
            com_step_size_mean = _get_mean_between_epochs(com_step_sizes, between)

            fout.write(f"{sim_id:>2d}  {com_step_size_mean: 12.8f}\n")


def write_converged_centre_of_mass_step_size_last(
    reader: ProjectDataReader,
    output_filepath: Path,
    sim_ids: Sequence[int],
    n_last: int,
) -> None:
    with open(output_filepath, "w") as fout:
        for sim_id in sim_ids:
            com_step_sizes = reader.read_project_centre_of_mass_step_size(sim_id)
            com_step_size_mean = _get_mean_last_n_epochs(com_step_sizes, n_last)

            fout.write(f"{sim_id:>2d}  {com_step_size_mean: 12.8f}\n")


def read_converged_bisection_multibead_position_move_info(filepath: Path) -> dict[int, MultibeadPositionMoveInfo]:
    # NOTE: it is not guaranteed that the simulation IDs are from 0 to some maximum integer, which is
    # why we use a dictionary instead of a list
    output: dict[int, MultibeadPositionMoveInfo] = {}

    with open(filepath, "r") as in_stream:
        for line in in_stream:
            tokens = line.split()
            sim_id = int(tokens[0])
            upper_level_fraction = float(tokens[1])
            lower_level = int(tokens[2])

            output[sim_id] = MultibeadPositionMoveInfo(upper_level_fraction, lower_level)

    return output


def read_converged_centre_of_mass_step_size(filepath: Path) -> dict[int, float]:
    # NOTE: it is not guaranteed that the simulation IDs are from 0 to some maximum integer, which is
    # why we use a dictionary instead of a list
    output: dict[int, float] = {}

    with open(filepath, "r") as in_stream:
        for line in in_stream:
            tokens = line.split()
            sim_id = int(tokens[0])
            com_step_size = float(tokens[1])

            output[sim_id] = com_step_size

    return output
