"""
Many of the estimated properties that are produced by the C++ simulation code follow a
simple format, where there are leading comments, followed by two columns:
- the 0th column is the epoch number
- the 1st column is the estimated value in the form of a floating-point number

------------
# comment explaining what the results are
# maybe another comment?
00005      1.23456
00006      2.34567
00007      3.45678
00008      4.56789
...
------------

This module contains code for handling information from these types of files.
"""

import dataclasses
from pathlib import Path
from typing import TextIO
from typing import Union

import numpy as np
from numpy.typing import NDArray


@dataclasses.dataclass
class PropertyData:
    epochs: NDArray[np.int32]
    values: NDArray[np.float64]


@dataclasses.dataclass
class PropertyStatistics:
    first_sampled_epoch: int
    n_samples: int
    mean: float
    stddev: float
    stderrmean: float


def read_property_data(filepath: Path, *, normalize_by: Union[int, float] = 1.0) -> PropertyData:
    with open(filepath, "r") as fin:
        return read_property_data_stream(fin, normalize_by=normalize_by)


def read_property_data_stream(stream: TextIO, *, normalize_by: Union[int, float] = 1.0) -> PropertyData:
    data = np.loadtxt(stream, comments="#", dtype={"names": ("epochs", "values"), "formats": (np.int32, np.float64)})
    return PropertyData(data["epochs"], data["values"] / float(normalize_by))


def get_property_statistics(data: PropertyData) -> PropertyStatistics:
    first_sampled_epoch = data.epochs[0]
    n_samples = data.epochs.size
    mean = np.average(data.values).astype(float)
    stddev = np.std(data.values).astype(float)
    stderrmean = stddev / np.sqrt(n_samples)

    return PropertyStatistics(first_sampled_epoch, n_samples, mean, stddev, stderrmean)
