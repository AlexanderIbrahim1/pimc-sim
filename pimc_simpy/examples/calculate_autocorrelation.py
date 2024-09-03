"""
This script is an example of calculating the autocorrelation time for data from
an MCMC simulation.
"""

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

from pimc_simpy.statistics import autocorrelation1d
from pimc_simpy.statistics import autocorrelation_time_from_data


def main() -> None:
    filename = "kinetic.dat"
    filepath = Path("..", "..", "playground", "ignore", filename)

    data = np.loadtxt(filepath, usecols=(1,))

    autocorrelation = autocorrelation1d(data)
    auto_time = autocorrelation_time_from_data(data)

    print(auto_time)

    fig = plt.figure()
    ax = fig.add_subplot()

    ax.plot(autocorrelation)
    plt.show()


if __name__ == "__main__":
    main()
