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
from typing import Any
from typing import Callable
from typing import Union

import numpy as np
from numpy.typing import NDArray

NumpyInteger = Union[np.int8, np.int16, np.int32, np.int64]
NumpyFloat = Union[np.float16, np.float32, np.float64, np.float128]
NumpyScalar = Union[NumpyInteger, NumpyFloat]


@dataclasses.dataclass
class PropertyData:
    epochs: NDArray[np.int32]
    values: NDArray[NumpyScalar]

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


def get_property_statistics(data: PropertyData) -> PropertyStatistics:
    first_sampled_epoch = data.epochs[0]
    n_samples = data.epochs.size
    mean = np.average(data.values).astype(float)
    stddev = np.std(data.values).astype(float)
    stderrmean = stddev / np.sqrt(n_samples)

    return PropertyStatistics(first_sampled_epoch, n_samples, mean, stddev, stderrmean)


def element_by_index(index: int, data: PropertyData) -> NumpyScalar:
    return data.values[index]


def element_by_epoch(epoch: int, data: PropertyData) -> NumpyScalar:
    index = _index_from_epoch_bisect(epoch, data)
    return data.values[index]


def between_indices(i_left: int, i_right: int, data: PropertyData) -> PropertyData:
    new_epochs = data.epochs[i_left:i_right]
    new_values = data.values[i_left:i_right]

    return PropertyData(new_epochs, new_values)


def between_epochs(i_epoch_left: int, i_epoch_right: int, data: PropertyData) -> PropertyData:
    i_left = _index_from_epoch_bisect(i_epoch_left, data, throw_if_at_end=True)
    i_right = _index_from_epoch_bisect(i_epoch_right, data, throw_if_at_end=False)

    return between_indices(i_left, i_right, data)


def last_n_epochs(n_epochs: int, data: PropertyData) -> PropertyData:
    i_epoch_right = data.epochs[-1]
    i_epoch_left = i_epoch_right - n_epochs

    return between_epochs(i_epoch_left, i_epoch_right, data)


def _index_from_epoch_bisect(epoch: int, data: PropertyData, *, throw_if_at_end: bool = True) -> int:
    i_left_bisect = np.searchsorted(data.epochs, epoch)
    if i_left_bisect < data.epochs.size and data.epochs[i_left_bisect] == epoch:
        return int(i_left_bisect)
    elif i_left_bisect == data.epochs.size and not throw_if_at_end:
        return int(i_left_bisect)
    else:
        raise ValueError(f"Epoch '{epoch}' not found in PropertyData instance")
