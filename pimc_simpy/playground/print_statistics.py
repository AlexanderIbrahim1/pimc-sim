"""
This script shows an example of loading a single value estimate file from the C++
simulation code, and printing out the results
"""

from pathlib import Path

from pimc_simpy.single_value_estimate import read_property_data
from pimc_simpy.single_value_estimate import get_property_statistics


def main() -> None:
    filepath = Path("kinetic.dat")
    n_particles = 180

    kinetic_data = read_property_data(filepath, normalize_by=n_particles)
    kinetic_statistics = get_property_statistics(kinetic_data)

    print(kinetic_statistics)


if __name__ == "__main__":
    main()
