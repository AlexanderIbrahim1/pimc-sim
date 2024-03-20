"""
This script shows an example of loading a single value estimate file from the C++
simulation code, and printing/plotting out the results.
"""

from pathlib import Path

from pimc_simpy.single_value_estimate import read_property_data
from pimc_simpy.single_value_estimate import get_property_statistics
from pimc_simpy.single_value_estimate import plot_property


def main() -> None:
    n_particles = 180

    kinetic_data = read_property_data(Path("kinetic.dat"), normalize_by=n_particles)
    kinetic_statistics = get_property_statistics(kinetic_data)

    pair_potential_data = read_property_data(Path("pair_potential.dat"), normalize_by=n_particles)
    pair_potential_statistics = get_property_statistics(pair_potential_data)

    plot_property([kinetic_data, pair_potential_data], labels=["kinetic", "pair potential"])


if __name__ == "__main__":
    main()
