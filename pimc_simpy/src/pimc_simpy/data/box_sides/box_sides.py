"""
This module contains the BoxSides class, for reading the sides of the box
in a simulation with periodic boundary conditions.
"""

from pathlib import Path
from typing import TextIO

from pimc_simpy._utils import skip_lines_that_start_with


class BoxSides:
    def __init__(self, *sides: float) -> None:
        self._sides: list[float] = []

        for side in sides:
            if side <= 0.0:
                raise ValueError("Found a non-positive side for the BoxSides constructor.")
            self._sides.append(side)

    def n_dimensions(self) -> int:
        return len(self._sides)

    def __getitem__(self, index: int) -> float:
        return self._sides[index]

    def __setitem__(self, index: int, value: float) -> None:
        self._sides[index] = value


def read_box_sides(filepath: Path) -> BoxSides:
    with open(filepath, "r") as in_stream:
        return read_box_sides_stream(in_stream)


def read_box_sides_stream(in_stream: TextIO) -> BoxSides:
    sides: list[float] = []

    skip_lines_that_start_with(in_stream, "#")

    n_dimensions = int(in_stream.readline().strip())
    for _ in range(n_dimensions):
        side = float(in_stream.readline().strip())
        sides.append(side)

    return BoxSides(*sides)
