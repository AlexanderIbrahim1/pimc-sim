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
from pimc_simpy.quick_analysis._read_project_data_common import read_project_bisection_multibead_position_move_info
from pimc_simpy.quick_analysis._read_project_data_common import read_project_centre_of_mass_step_size
from pimc_simpy.manage import ProjectInfo


def _get_mean_between_epochs(data: PropertyData, between: tuple[int, int]) -> float:
    data = between_epochs(between[0], between[1], data)
    stats = get_property_statistics(data)
    return stats.mean


def _get_mean_last_n_epochs(data: PropertyData, last_n: int) -> float:
    data = last_n_epochs(last_n, data)
    stats = get_property_statistics(data)
    return stats.mean


def _parse_between_argument(between: Union[tuple[int, int], Sequence[tuple[int, int]]], n_sim_ids: int) -> list[tuple[int, int]]:
    is_tuple_pair = isinstance(between, tuple) and isinstance(between[0], int) and len(between) == 2

    if is_tuple_pair:
        return [between for _ in range(n_sim_ids)]
    else:
        # assume it is `Sequence[tuple[int, int]]`
        if len(between) != n_sim_ids:
            raise ValueError("The number of pairs of tuples in `between` must match the number of sim IDs.")
        return list(between)


def _parse_n_last_argument(n_last: Union[int, Sequence[int]], n_sim_ids: int) -> list[tuple[int, int]]:
    if isinstance(n_last, int):
        return [n_last for _ in range(n_sim_ids)]
    else:
        # assume it is `Sequence[int]`
        if len(n_last) != n_sim_ids:
            raise ValueError("The number of pairs of tuples in `between` must match the number of sim IDs.")
        return list(n_last)


def write_converged_bisection_multibead_position_move_info_between(
    info: ProjectInfo,
    output_filepath: Path,
    sim_ids: Sequence[int],
    between: Union[tuple[int, int], Sequence[tuple[int, int]]],
) -> None:
    between = _parse_between_argument(between, len(sim_ids))

    with open(output_filepath, "w") as fout:
        for sim_id, epoch_pair in zip(sim_ids, between):
            upper_fractions, lower_levels = read_project_bisection_multibead_position_move_info(info, sim_id)

            upper_fraction_mean = _get_mean_between_epochs(upper_fractions, epoch_pair)
            lower_level_mean = round(_get_mean_between_epochs(lower_levels, epoch_pair))

            fout.write(f"{sim_id:>2d}  {upper_fraction_mean: 12.8f}  {lower_level_mean:>2d}\n")


def write_converged_bisection_multibead_position_move_info_last(
    info: ProjectInfo,
    output_filepath: Path,
    sim_ids: Sequence[int],
    n_last: Union[int, Sequence[int]],
) -> None:
    n_last = _parse_n_last_argument(n_last, len(sim_ids))

    with open(output_filepath, "w") as fout:
        for sim_id, n_epochs in zip(sim_ids, n_last):
            upper_fractions, lower_levels = read_project_bisection_multibead_position_move_info(info, sim_id)

            upper_fraction_mean = _get_mean_last_n_epochs(upper_fractions, n_epochs)
            lower_level_mean = round(_get_mean_last_n_epochs(lower_levels, n_epochs))

            fout.write(f"{sim_id:>2d}  {upper_fraction_mean: 12.8f}  {lower_level_mean:>2d}\n")


def write_converged_centre_of_mass_step_size_between(
    info: ProjectInfo,
    output_filepath: Path,
    sim_ids: Sequence[int],
    between: Union[tuple[int, int], Sequence[tuple[int, int]]],
) -> None:
    between = _parse_between_argument(between, len(sim_ids))

    with open(output_filepath, "w") as fout:
        for sim_id, epoch_pair in zip(sim_ids, between):
            com_step_sizes = read_project_centre_of_mass_step_size(info, sim_id)
            com_step_size_mean = _get_mean_between_epochs(com_step_sizes, epoch_pair)

            fout.write(f"{sim_id:>2d}  {com_step_size_mean: 12.8f}\n")


def write_converged_centre_of_mass_step_size_last(
    info: ProjectInfo,
    output_filepath: Path,
    sim_ids: Sequence[int],
    n_last: Union[int, Sequence[int]],
) -> None:
    n_last = _parse_n_last_argument(n_last, len(sim_ids))

    with open(output_filepath, "w") as fout:
        for sim_id, n_epochs in zip(sim_ids, n_last):
            com_step_sizes = read_project_centre_of_mass_step_size(info, sim_id)
            com_step_size_mean = _get_mean_last_n_epochs(com_step_sizes, n_epochs)

            fout.write(f"{sim_id:>2d}  {com_step_size_mean: 12.8f}\n")


def read_converged_bisection_multibead_position_move_info(filepath: Path) -> list[tuple[int, MultibeadPositionMoveInfo]]:
    output: list[tuple[int, MultibeadPositionMoveInfo]] = []

    with open(filepath, "r") as in_stream:
        for line in in_stream:
            tokens = line.split()
            sim_id = int(tokens[0])
            upper_level_fraction = float(tokens[1])
            lower_level = int(tokens[2])

            output.append((sim_id, MultibeadPositionMoveInfo(upper_level_fraction, lower_level)))

    return output


def read_converged_centre_of_mass_step_size(filepath: Path) -> list[tuple[int, float]]:
    output: list[tuple[int, float]] = []

    with open(filepath, "r") as in_stream:
        for line in in_stream:
            tokens = line.split()
            sim_id = int(tokens[0])
            com_step_size = float(tokens[1])

            output.append((sim_id, com_step_size))

    return output
