"""
This module contains code for working with and plotting radial distribution functions
and centroid radial distribution functions.
"""

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.data import HistogramInfo
from pimc_simpy.data import create_bin_info


def normalized_radial_distribution_function(
    histogram: HistogramInfo, *, n_groups: int = 1
) -> tuple[NDArray[np.float64], NDArray[np.float64]]:
    """
    Create the normalized radial distribution function g(r) from the information about
    the histogram counts from the simulation data. This function also generates the
    centroid radial distribution functions.

    This function, of course, assumes that the results from the simulation data were
    estimated with an r^2 distance scaling (i.e. the original histogram actually
    collects ~ g(r) * r^2).

    n_groups: int
        - the number of adjacent bins in the original histogram to combine into a single bin

    returns (distance_centres, radial_dist_func)
        - distance_centres: the r-value for the centre of each histogram bin
        - radial_dist_func: the corresponding normalized g(r) for each r in the distance centres
    """
    bin_info = create_bin_info(histogram, n_groups=n_groups)

    distance_centres = (bin_info.bin_edges[1:] + bin_info.bin_edges[:-1]) / 2.0

    distances_squared = distance_centres**2
    distances_squared /= np.trapz(distances_squared)

    # convert to float64 to avoid numpy.core._exceptions._UFuncOutputCastingError when dividing by np.trapz(counts)
    counts = np.array(bin_info.counts, dtype=np.float64)
    counts /= np.trapz(counts)

    radial_dist_func = counts / distances_squared

    return distance_centres, radial_dist_func
