"""
This script takes a worldline file (.xyz) from the old moribs code, and converts it to a
format suitable for the current pimc-sim code.
"""

from pathlib import Path
from typing import TextIO

from cartesian3d import Cartesian3D
from cartesian3d import format_cartesian3d_for_pimc_sim_worldline
from cartesian3d import parse_into_cartesian3d


# PLAN
# - read in the .xyz file as a list of (x, y, z) points
#   - remember to skip the first two lines
# - rearrange into worldline-contiguous format
# - rewrite into format that pimc-sim uses


def read_moribs_worldline_file(filestream: TextIO) -> list[Cartesian3D]:
    # the first two lines are an (unneeded) integer and a comment
    filestream.readline()
    filestream.readline()

    points: list[Cartesian3D] = []
    for line in filestream:
        point = parse_into_cartesian3d(line)
        points.append(point)

    return points


def rearrange_into_worldlines(points: list[Cartesian3D], n_worldlines: int, n_particles: int) -> list[list[Cartesian3D]]:
    assert n_worldlines > 0
    assert n_particles > 0
    assert len(points) == n_worldlines * n_particles

    worldlines: list[list[Cartesian3D]] = [[] for _ in range(n_worldlines)]

    for i, point in enumerate(points):
        i_worldline = i % n_worldlines
        worldlines[i_worldline].append(point)

    return worldlines


def write_pimc_sim_worldline_file(filestream: TextIO, worldlines: list[list[Cartesian3D]], i_block: int) -> None:
    header_contents_filename = "worldline_file_header_contents.txt"
    with open(header_contents_filename, "r") as fin:
        header = fin.read()

    n_dimensions = 3
    n_particles = len(worldlines[0])
    n_worldlines = len(worldlines)

    filestream.write(f"{header}")
    filestream.write(f"{i_block}\n")
    filestream.write(f"{n_dimensions}\n")
    filestream.write(f"{n_particles}\n")
    filestream.write(f"{n_worldlines}\n")

    for worldline in worldlines:
        for bead in worldline:
            line = format_cartesian3d_for_pimc_sim_worldline(bead)
            filestream.write(f"{line}\n")


if __name__ == "__main__":
    worldline_dirpath = Path("..", "..", "..", "other", "investigations", "moribs_pimc_conversion", "example4")
    original_filepath = worldline_dirpath / "gr00002.xyz"
    i_block = 2
    n_particles = 180
    n_worldlines = 64

    with open(original_filepath, "r") as filestream:
        points = read_moribs_worldline_file(filestream)
        worldlines = rearrange_into_worldlines(points, n_worldlines, n_particles)

    new_filepath = worldline_dirpath / "worldline00002.dat"

    with open(new_filepath, "w") as outstream:
        write_pimc_sim_worldline_file(outstream, worldlines, i_block)
