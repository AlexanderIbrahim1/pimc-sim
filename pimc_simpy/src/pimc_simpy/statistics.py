"""
This module contains functions for calculating certain types of statistical values.
"""

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.data.property_data import PropertyData


def calculate_cumulative_means_and_sems(data: PropertyData) -> tuple[NDArray[np.float64], NDArray[np.float64]]:
    n_samples = data.epochs.size

    cumulative_means = np.empty(n_samples, dtype=np.float64)
    cumulative_sems = np.empty(n_samples, dtype=np.float64)

    for i in range(n_samples):
        i_upper = i + 1
        cumulative_values = data.values[:i_upper]
        mean = np.average(cumulative_values).astype(np.float64)
        stddev = np.std(cumulative_values).astype(np.float64)
        sem = stddev / np.sqrt(i_upper)

        cumulative_means[i] = mean
        cumulative_sems[i] = sem

    return cumulative_means, cumulative_sems
