"""
This script contains code for collecting information about the optimal Monte Carlo
step size information from the simulations run so far.
"""

from pathlib import Path

import numpy as np

from pimc_simpy.data import read_property_data
from pimc_simpy.data import read_property_data_multiple
from pimc_simpy.manage import get_abs_job_output_dirpath
from pimc_simpy.data import between_epochs

from project_info import ProjectInfo


BISECTION_MOVE_INFO_FILENAME: str = "bisection_multibead_position_move_info.dat"
CENTRE_OF_MASS_STEP_SIZE_FILENAME: str = "centre_of_mass_step_size.dat"


def get_n_particles_in_hcp(cells: tuple[int, int, int]) -> int:
    """Return the number of particles in the box, assuming an HCP lattice."""
    n_particles_in_cell: int = 4

    return cells[0] * cells[1] * cells[2] * n_particles_in_cell


def collect_com_step_size_info(abs_job_output_dirpath: Path, n_last_to_average: int) -> float:
    filepath = abs_job_output_dirpath / CENTRE_OF_MASS_STEP_SIZE_FILENAME

    com_data = read_property_data(filepath, dtype=np.float64)

    i_epoch_right = com_data.epochs[-1]
    i_epoch_left = i_epoch_right - n_last_to_average
    com_data = between_epochs(i_epoch_left, i_epoch_right, com_data)

    return np.average(com_data.values).astype(float)


def collect_all_com_step_size_info() -> None:
    n_densities: int = 31
    info = ProjectInfo()

    com_step_sizes = np.empty(n_densities, dtype=np.float64)

    out_filepath = Path("ideal_mc_steps", "ideal_com_step_sizes.dat")

    for sim_id in range(n_densities):
        abs_job_output_dirpath = get_abs_job_output_dirpath(info, sim_id)
        com_step_sizes[sim_id] = collect_com_step_size_info(abs_job_output_dirpath, 100)

    np.savetxt(out_filepath, com_step_sizes)


def collect_bisection_step_size_info(abs_job_output_dirpath: Path, n_last_to_average: int) -> tuple[float, int]:
    filepath = abs_job_output_dirpath / BISECTION_MOVE_INFO_FILENAME

    upper_fractions, lower_levels = read_property_data_multiple(filepath, n_data=2, dtypes=(np.float64, np.int32))

    i_epoch_right = upper_fractions.epochs[-1]
    i_epoch_left = i_epoch_right - n_last_to_average

    upper_fractions = between_epochs(i_epoch_left, i_epoch_right, upper_fractions)
    lower_levels = between_epochs(i_epoch_left, i_epoch_right, lower_levels)

    average_upper_fraction = np.average(upper_fractions.values).astype(float)
    average_lower_level = np.average(lower_levels.values).astype(int)

    return average_upper_fraction, average_lower_level


def collect_all_bisection_step_size_info() -> None:
    n_densities: int = 31
    info = ProjectInfo()

    out_filepath = Path("ideal_mc_steps", "ideal_bisection_step_sizes.dat")

    with open(out_filepath, "w") as fout:
        for sim_id in range(n_densities):
            abs_job_output_dirpath = get_abs_job_output_dirpath(info, sim_id)
            upper_fraction, lower_level = collect_bisection_step_size_info(abs_job_output_dirpath, 100)
            fout.write(f"{upper_fraction: 12.8f}   {lower_level}\n")


if __name__ == "__main__":
    collect_all_bisection_step_size_info()
