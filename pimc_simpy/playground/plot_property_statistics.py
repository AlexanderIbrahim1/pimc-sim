"""
This script shows an example of loading a single value estimate file from the C++
simulation code, and printing/plotting out the results.
"""

from pathlib import Path

import numpy as np

from pimc_simpy.data import read_property_data
from pimc_simpy.data import read_property_data_multiple
from pimc_simpy.data import get_property_statistics
from pimc_simpy.plotting import plot_property
from pimc_simpy.plotting import plot_property_cumulative
from pimc_simpy.plotting import plot_property_rescaled


def main() -> None:
    n_particles = 180

    kinetic_data = read_property_data(Path("kinetic.dat"), normalize_by=n_particles, dtype=float)
    kinetic_statistics = get_property_statistics(kinetic_data)

    pair_potential_data = read_property_data(Path("pair_potential.dat"), normalize_by=n_particles)
    pair_potential_statistics = get_property_statistics(pair_potential_data)

    plot_property_rescaled([kinetic_data, pair_potential_data], labels=["kinetic", "pair potential"])


#     com_move_accept_filepath = Path("../../playground/ignore_with_timer/centre_of_mass_position_move_accept.dat")
#     accepts, rejects = read_property_data_multiple(com_move_accept_filepath, 2, normalize_by=(2, 3), dtypes=(float, float))
#
#     print(accepts)
#     print(rejects)


if __name__ == "__main__":
    main()
