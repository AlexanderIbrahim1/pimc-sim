"""
This subpackage contains functions for finding the relevant statistics of data collected
from the simulation output files.
"""

from pimc_simpy.statistics.cumulative import calculate_cumulative_means_and_sems
from pimc_simpy.statistics.autocorrelation import autocorrelation1d
from pimc_simpy.statistics.autocorrelation import autocorrelation_time_from_data
from pimc_simpy.statistics.autocorrelation import autocorrelation_time_from_autocorrelations
