"""
This script contains code to read in the data from the original FSH potential file
(in units of angstroms and Kelvin), convert the energies to wavenumbers, and write
them out again.
"""

from pathlib import Path
import numpy as np

WAVENUMBER_PER_KELVIN: float = 0.6950302506112777


def main() -> None:
    input_filename = Path("..", "..", "potentials", "h2h2v00_fsh.potext_sq")
    pair_distances_squared, energies = np.loadtxt(input_filename, usecols=(0, 1), unpack=True)
    
    energies *= WAVENUMBER_PER_KELVIN
    
    output_filename = Path("..", "..", "potentials", "fsh_potential_angstroms_wavenumbers.potext_sq")
    output_data = np.vstack((pair_distances_squared, energies)).T
    np.savetxt(output_filename, output_data)



if __name__ == "__main__":
    main()
