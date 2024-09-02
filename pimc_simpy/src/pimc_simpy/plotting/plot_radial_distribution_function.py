"""
This module contains code for working with and plotting radial distribution functions
and centroid radial distribution functions.
"""

import matplotlib.pyplot as plt
import numpy as np

from pimc_simpy.data import HistogramInfo
from pimc_simpy.data import create_bin_info
from pimc_simpy.radial_distribution_function import normalized_radial_distribution_function

_RADIAL_DISTRIBUTION_FUNCTION_UPPER_BOUND_PLOT_RATIO: float = 1.1


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
    # NOTE: we repeat the bin_info calculation, but it isn't really a performance issue, so w/e
    bin_info = create_bin_info(histogram, n_groups=n_groups)
    _, norm_gr = normalized_radial_distribution_function(histogram, n_groups=n_groups)

    fig = plt.figure()
    ax = fig.add_subplot()

    y_plot_min = 0.0
    y_plot_max = np.max(norm_gr).astype(float) * _RADIAL_DISTRIBUTION_FUNCTION_UPPER_BOUND_PLOT_RATIO

    ax.set_xlim(histogram.minimum, histogram.maximum)
    ax.set_ylim(y_plot_min, y_plot_max)

    # reason for ignore: NDArray[np.float64] not recognized as Sequence[float], which ax.hist asks for when setting `bins`
    ax.hist(x=bin_info.bin_left_edges, bins=bin_info.bin_edges, weights=norm_gr)  # type: ignore
    plt.show()
