"""
This module contains components for handling the histogram outputs from the C++ project.
"""

import dataclasses
import enum
from pathlib import Path
from typing import TextIO

import numpy as np
from numpy.typing import NDArray

from pimc_simpy._utils import skip_lines_that_start_with


class OutOfRangePolicy(enum.Enum):
    DO_NOTHING = enum.auto()
    THROW = enum.auto()


@dataclasses.dataclass
class HistogramInfo:
    policy: OutOfRangePolicy
    n_bins: int
    minimum: float
    maximum: float
    bins: NDArray


_POLICY_ID_TO_POLICY: list[OutOfRangePolicy] = [
    OutOfRangePolicy.DO_NOTHING,
    OutOfRangePolicy.THROW,
]


def read_histogram(filepath: Path) -> HistogramInfo:
    with open(filepath, "r") as fin:
        return read_histogram_stream(fin)


def read_histogram_stream(fin: TextIO) -> HistogramInfo:
    skip_lines_that_start_with(fin, "#")

    # read the policy
    policy_id = int(fin.readline().strip())
    policy = _POLICY_ID_TO_POLICY[policy_id]

    # read the number of bins, minimum value, and maximum value
    n_bins = int(fin.readline().strip())
    minimum = float(fin.readline().strip())
    maximum = float(fin.readline().strip())

    # read the bin entries
    bins = np.empty(n_bins, dtype=int)
    for i in range(n_bins):
        bins[i] = int(fin.readline().strip())

    return HistogramInfo(policy, n_bins, minimum, maximum, bins)
