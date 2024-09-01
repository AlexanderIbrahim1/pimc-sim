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

from __future__ import annotations

import dataclasses
from collections.abc import Sequence
from pathlib import Path
from typing import Any
from typing import Callable
from typing import Optional
from typing import TextIO
from typing import Union

import matplotlib.pyplot as plt
import numpy as np
from numpy.typing import NDArray


@dataclasses.dataclass
class PropertyData:
    epochs: NDArray[np.int32]
    values: NDArray[np.float64]

    def __add__(self, other: Any) -> PropertyData:
        return self._operation_with_number(other, lambda x, y: x + y)

    def __radd__(self, other: Any) -> PropertyData:
        return self.__add__(other)

    def __sub__(self, other: Any) -> PropertyData:
        return self._operation_with_number(other, lambda x, y: x - y)

    def __rsub__(self, other: Any) -> PropertyData:
        return self.__sub__(other)

    def __mul__(self, other: Any) -> PropertyData:
        return self._operation_with_number(other, lambda x, y: x * y)

    def __rmul__(self, other: Any) -> PropertyData:
        return self.__mul__(other)

    def __truediv__(self, other: Any) -> PropertyData:
        return self._operation_with_number(other, lambda x, y: x / y)

    def __rtruediv__(self, other: Any) -> PropertyData:
        return self.__truediv__(other)

    def __floordiv__(self, other: Any) -> PropertyData:
        return self._operation_with_number(other, lambda x, y: x // y)

    def __rfloordiv__(self, other: Any) -> PropertyData:
        return self.__floordiv__(other)

    def _operation(self, other: Any, operation: Callable[[Any, Any], Any]) -> PropertyData:
        if not isinstance(other, PropertyData):
            return NotImplemented

        self._check_epochs(other.epochs)
        return PropertyData(self.epochs, operation(self.values, other.values))

    def _operation_with_number(self, other: Any, operation: Callable[[Any, Any], Any]) -> PropertyData:
        if isinstance(other, PropertyData):
            self._check_epochs(other.epochs)
            return PropertyData(self.epochs, operation(self.values, other.values))
        elif isinstance(other, float) or isinstance(other, int):
            return PropertyData(self.epochs, operation(float(other), self.values))
        else:
            return NotImplemented

    def _check_epochs(self, other_epochs: NDArray[np.int32]) -> None:
        if not np.array_equal(self.epochs, other_epochs):
            raise ValueError("Cannot add two properties not collected over the same epochs.")


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


def plot_property(properties: Union[PropertyData, Sequence[PropertyData]], *, labels: Optional[Sequence[str]] = None) -> None:
    if isinstance(properties, PropertyData):
        if labels is not None:
            labels = labels[0]

        plot_single_property(properties, label=labels)
        return

    if len(properties) == 0:
        raise ValueError("There must be at least one type of data to plot.")

    if labels is not None and len(labels) != len(properties):
        raise ValueError("The number of labels must match the number of types of data to plot.")

    fig = plt.figure()
    ax = fig.add_subplot()

    x_lim_min = properties[0].epochs[0]
    x_lim_max = properties[0].epochs[-1]
    ax.set_xlim(x_lim_min, x_lim_max)

    for i, data in enumerate(properties):
        cumulative_means, cumulative_sems = _calculate_cumulative_means_and_sems(data)

        if labels is not None:
            label = labels[i]
            ax.plot(data.epochs, cumulative_means, label=label)
        else:
            ax.plot(data.epochs, cumulative_means)

        ax.fill_between(data.epochs, cumulative_means + cumulative_sems, cumulative_means - cumulative_sems, alpha=0.5)

        ax.legend()

    plt.show()


def plot_single_property(data: PropertyData, *, label: Optional[str] = None) -> None:
    cumulative_means, cumulative_sems = _calculate_cumulative_means_and_sems(data)

    fig = plt.figure()
    ax = fig.add_subplot()

    ax.set_xlim(data.epochs[0], data.epochs[-1])

    if label is not None:
        ax.plot(data.epochs, cumulative_means, label=label)
    else:
        ax.plot(data.epochs, cumulative_means)

    ax.fill_between(data.epochs, cumulative_means + cumulative_sems, cumulative_means - cumulative_sems, alpha=0.5)

    plt.show()


def _calculate_cumulative_means_and_sems(data: PropertyData) -> tuple[NDArray[np.float64], NDArray[np.float64]]:
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
