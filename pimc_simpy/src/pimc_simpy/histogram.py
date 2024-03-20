"""
This module contains components for handling the histogram outputs from the C++ project.
"""

import dataclasses
import enum
from pathlib import Path
from typing import TextIO

import matplotlib.pyplot as plt
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
    counts: NDArray


@dataclasses.dataclass
class BinInfo:
    bin_left_edges: NDArray[np.float64]
    bin_edges: NDArray[np.float64]
    counts: NDArray[np.int32]


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


def create_bin_info(hist_info: HistogramInfo, *, n_groups: int = 1) -> BinInfo:
    if hist_info.n_bins % n_groups != 0:
        raise ValueError("The number of bins needs to be divisible by `n_groups`")

    n_bins = hist_info.n_bins // n_groups
    bin_widths = (hist_info.maximum - hist_info.minimum) / n_bins
    bin_edges = np.array([hist_info.minimum + i * bin_widths for i in range(n_bins + 1)])
    bin_left_edges = bin_edges[:-1]

    bin_counts = np.empty(n_bins, dtype=int)
    for i in range(n_bins):
        i_left = i * n_groups
        i_right = i_left + n_groups
        bin_counts[i] = sum(hist_info.counts[i_left:i_right])

    return BinInfo(bin_left_edges, bin_edges, bin_counts)


def plot_radial_distribution_function(histogram: HistogramInfo, *, n_groups: int = 1) -> None:
    """
    Plot the normalized radial distribution function g(r) from the information about
    the histogram counts.

    n_groups: int
        - the number of adjacent bins in the original histogram to combine into a single bin

    NOTE: this function is not intended to produce professional-quality plots. Those tend
    to require the user to tweak several small details, so it would be infeasible to try
    to include all the possible things that different users might want.
    """
    bin_info = create_bin_info(histogram, n_groups=4)

    # the original histogram actually collects ~ g(r) * r^2
    r_squared = ((bin_info.bin_edges[1:] + bin_info.bin_edges[:-1]) / 2.0) ** 2
    r_squared /= np.trapz(r_squared)

    # convert to float64 to avoid numpy.core._exceptions._UFuncOutputCastingError
    counts = 1.0 * bin_info.counts
    counts /= np.trapz(counts)

    gr = counts / r_squared

    fig = plt.figure()
    ax = fig.add_subplot()

    ax.hist(x=bin_info.bin_left_edges, bins=bin_info.bin_edges, weights=gr)  # type: ignore
    plt.show()
