"""
This script shows an example of loading a histogram file produced by the C++
simulation code, and plotting the results.
"""

from pathlib import Path

from pimc_simpy.data import read_histogram
from pimc_simpy.plotting.plot_radial_distribution_function import plot_radial_distribution_function


def main() -> None:
    filepath = Path("radial_dist_histo.dat")
    histogram = read_histogram(filepath)
    plot_radial_distribution_function(histogram, n_groups=8)


if __name__ == "__main__":
    main()
