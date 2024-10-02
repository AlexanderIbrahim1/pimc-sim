"""
This file contains the Cartesian3D class, which is a POD type to hold points for the
worldline conversion process.
"""

import dataclasses


@dataclasses.dataclass
class Cartesian3D:
    x: float
    y: float
    z: float


def format_cartesian3d_for_pimc_sim_worldline(point: Cartesian3D) -> str:
    return f"{point.x: 12.8e}   {point.y: 12.8e}   {point.z: 12.8e}"


def parse_into_cartesian3d(line: str) -> Cartesian3D:
    tokens = line.split()
    x = float(tokens[0])
    y = float(tokens[1])
    z = float(tokens[2])

    return Cartesian3D(x, y, z)


if __name__ == "__main__":
    point = Cartesian3D(-3.263748882e-01, -5.127311941e-01, -4.627702832e-01)

    print(format_cartesian3d_for_pimc_sim_worldline(point))
    # -3.26374888e-01   -5.12731194e-01   -4.62770283e-01
