"""
This module contains the BinInfo class, which contains information about a histogram
that certain functions need in order to properly analyze the histogram.
"""

import dataclasses

import numpy as np
from numpy.typing import NDArray


from pimc_simpy.data.histogram.histogram import HistogramInfo


@dataclasses.dataclass
class BinInfo:
    bin_left_edges: NDArray[np.float64]
    bin_edges: NDArray[np.float64]
    counts: NDArray[np.int32]


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
