"""
Each job only performed the simulation for a single block, and thus only collected
a single block's worth of data for a histogram.

This script collects all the histogram data and creates a single histogram file.
"""

import textwrap
from io import TextIOWrapper
from pathlib import Path

from block_index_mapping import read_block_to_job_index_map
from common import get_abs_jobs_dirpath

# it's easier to hardcode these numbers, since I'll be using them only once
N_BINS: int = 1024
N_LINES_TO_SKIP: int = 11


def _get_header() -> str:
    header = textwrap.dedent(
        """
# This file contains the state of a regularly-spaced histogram
# The layout for the histogram data is as follows:
# - [integer] the out-of-range policy (0 = DO_NOTHING, 1 = THROW)
# - [integer] the number of bins
# - [floating-point] the minimum value
# - [floating-point] the maximum value
# ... followed by the count in each histogram bin, in single-column order...
0
1024
0.00000000e00
5.92353010e00
"""
    ).lstrip()

    return header


def add_to_histogram(instream: TextIOWrapper, histogram: list[int]) -> list[int]:
    for _ in range(N_LINES_TO_SKIP):
        instream.readline()

    for i in range(N_BINS):
        line = instream.readline()
        n_entries = int(line.strip())
        histogram[i] += n_entries


def main() -> None:
    # histogram_filename = "radial_dist_histo.dat"
    histogram_filename = "centroid_radial_dist_histo.dat"

    histogram: list[int] = [0 for _ in range(N_BINS)]

    block_to_job_index_map = read_block_to_job_index_map()
    abs_jobs_dirpath = get_abs_jobs_dirpath()

    for i_job in block_to_job_index_map.values():
        rel_histo_filepath = abs_jobs_dirpath / f"job_{i_job:0>3d}" / "output" / histogram_filename
        with open(rel_histo_filepath, "r") as instream:
            add_to_histogram(instream, histogram)

    output_filepath = Path(".", "output", histogram_filename)
    with open(output_filepath, "w") as fout:
        header = _get_header()
        fout.write(header)
        for entry in histogram:
            fout.write(f"{entry}\n")


if __name__ == "__main__":
    main()
