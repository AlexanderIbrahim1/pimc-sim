"""
This script is used to create smaller versions of the original three-body PES grid.

The original grid is 501 * 301 * 201 = ~3 x 10^7 entries. This takes a long time to
load, and isn't suitable for quick iterative checks. This script cuts down on the
size of the data grid, creating smaller ones that are faster to load and iterate with.
"""

from pathlib import Path

import numpy as np
from numpy.typing import NDArray


def load_three_body_potential_grid(filepath: Path) -> NDArray[np.float64]:
    return np.loadtxt(filepath, skiprows=19, dtype=np.float64).reshape(501, 301, 201)


def create_smaller_file() -> None:
    filepath = Path(".", "eng.tri")
    original = load_three_body_potential_grid(filepath)
    new_data = np.empty((126, 101, 51), dtype=np.float64)

    print(original[0, 0, 0])
    print(original[0, 0, 1])
    print(original[0, 0, 2])
    print(original[0, 0, 3])

    i_step = 4
    j_step = 3
    k_step = 4

    for i in range(0, 501, i_step):
        for j in range(0, 301, j_step):
            for k in range(0, 201, k_step):
                new_data[i // i_step, j // j_step, k // k_step] = original[i, j, k]

    output_filepath = Path(".", "threebody_126_101_51.dat")
    np.savetxt(output_filepath, new_data.flatten().T)


# 0.5 -> 5.5
# 1.0 -> 4.0
# 0.0 -> 1.0

if __name__ == "__main__":
    create_smaller_file()
