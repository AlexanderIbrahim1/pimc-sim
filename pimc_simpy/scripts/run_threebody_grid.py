"""
This script is used call the three-body potential using trilinear interpolation.
"""

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from numpy.typing import NDArray

from trilinear import TrilinearInterpolator


def load_energies() -> NDArray[np.float64]:
    # filepath = Path(".", "threebody_126_101_51.dat")
    filepath = Path(".", "eng.tri")

    # the C++ version should load the data from the file, but here we'll just fit the sizes
    # rsize = 126
    # ssize = 101
    # usize = 51
    rsize = 501
    ssize = 301
    usize = 201
    energies = np.loadtxt(filepath, skiprows=19, dtype=np.float64)
    energies = energies.reshape(rsize, ssize, usize)

    return energies


def main() -> None:
    energies = load_energies()

    rlims = (0.5, 5.5)
    slims = (1.0, 4.0)
    ulims = (0.0, 1.0)

    interpolator = TrilinearInterpolator(energies, rlims, slims, ulims)

    distances = np.linspace(0.5, 5.49, 256)
    energies = np.array([interpolator(r, 1.0, 0.99) for r in distances])

    fig = plt.figure()
    ax = fig.add_subplot()

    ax.plot(distances, energies)
    plt.show()


if __name__ == "__main__":
    main()
