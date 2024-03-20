from pathlib import Path

from pimc_simpy.histogram import read_histogram
from pimc_simpy.radial_distribution_function import plot_radial_distribution_function


def main() -> None:
    filepath = Path("radial_dist_histo.dat")
    histogram = read_histogram(filepath)
    plot_radial_distribution_function(histogram, n_groups=8)


if __name__ == "__main__":
    main()
