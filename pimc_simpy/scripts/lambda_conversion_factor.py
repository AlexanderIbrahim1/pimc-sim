"""
This script performs and checks the calculation of the conversion factor for the lambda
constant used in the PIMC simulation.
"""

from dataclasses import dataclass


@dataclass
class MassAMU:
    value: float


def lambda_conversion_coefficient() -> float:
    hbar = 1.054_571_817e-34  # J * s
    kilograms_per_amu = 1.660_540_200e-27  # kg * amu^{-1}
    boltzmann_constant = 1.380_649e-23  # J * K^{-1}
    angstroms_per_metre = 1.0e10  # ANG * m^{-1}

    coefficient = hbar * hbar / kilograms_per_amu / boltzmann_constant
    coefficient *= angstroms_per_metre**2

    return 0.5 * coefficient


def original_lambda(mass: MassAMU) -> float:
    hbar = 1.054_571_817e-34  # J * s
    kilograms_per_amu = 1.660_540_200e-27  # kg * amu^{-1}
    boltzmann_constant = 1.380_649e-23  # J * K^{-1}
    angstroms_per_metre = 1.0e10  # ANG * m^{-1}

    coefficient = hbar * hbar / kilograms_per_amu / boltzmann_constant
    coefficient *= angstroms_per_metre**2

    return 0.5 * coefficient / mass.value


def new_lambda(mass: MassAMU) -> float:
    hbar = 1.054_571_817
    kilograms_per_amu = 1.660_540_200
    boltzmann_constant = 1.380_649

    expon_coeff = 100.0

    coefficient = expon_coeff * hbar * hbar / kilograms_per_amu / boltzmann_constant

    return 0.5 * coefficient / mass.value


if __name__ == "__main__":
    mass = MassAMU(2.015650642)
    value = new_lambda(mass)
    print(value)
    print(lambda_conversion_coefficient())
