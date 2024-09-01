"""
This subpackage contains code for dealing with histogram data from the simulation output.
"""

from pimc_simpy.data.histogram.histogram import read_histogram
from pimc_simpy.data.histogram.histogram import read_histogram_stream
from pimc_simpy.data.histogram.histogram import HistogramInfo
from pimc_simpy.data.histogram.histogram import OutOfRangePolicy

from pimc_simpy.data.histogram.bin_info import create_bin_info
from pimc_simpy.data.histogram.bin_info import BinInfo
