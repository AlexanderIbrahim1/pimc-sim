"""
Many of the estimated properties that are produced by the C++ simulation code follow a
simple format, where there are leading comments, followed by multiple columns:
- the 0th column is the epoch number
- the 1st column is some value in the form of a number (floating-point or integer)
- the 2st column is some value in the form of a number (floating-point or integer)
...
etc.

There are at least two columns

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

from pathlib import Path
from typing import Optional
from typing import Type
from typing import TextIO

import numpy as np

from pimc_simpy.data.property_data import PropertyData

_EPOCH_NAME: str = "epoch"


def read_property_data_multiple(
    filepath: Path,
    n_data: int,
    *,
    normalize_by: Optional[tuple[float, ...]] = None,
    dtypes: Optional[tuple[Type, ...]] = None,
) -> list[PropertyData]:
    """
    n_data: int
        - the number of columns in the file EXPECT FOR the column for the epoch number labels
        - for example, if the file's contents look like the following, then `n_data == 1`
          ------------
          # comment explaining what the results are
          # maybe another comment?
          00005      1.23456
          00006      2.34567
          00007      3.45678
          00008      4.56789
          ...
          ------------
    """
    if n_data < 1:
        raise ValueError("At least one column of data must be read from the file.")

    with open(filepath, "r") as fin:
        return _read_property_data_stream(fin, n_data, normalize_by=normalize_by, dtypes=dtypes)


def read_property_data(
    filepath: Path,
    *,
    normalize_by: float = 1.0,
    dtype: Type = np.float64,
) -> PropertyData:
    with open(filepath, "r") as fin:
        property_datas = _read_property_data_stream(fin, n_data=1, normalize_by=(normalize_by,), dtypes=(dtype,))
        return property_datas[0]


def _read_property_data_stream(
    stream: TextIO,
    n_data: int,
    *,
    normalize_by: Optional[tuple[float, ...]] = None,
    dtypes: Optional[tuple[Type, ...]] = None,
) -> list[PropertyData]:
    dtype_names = _create_dtype_names(n_data)
    dtype_formats = _create_dtype_formats(n_data, dtypes)
    dtype_dict = {"names": dtype_names, "formats": dtype_formats}

    # append dummy norm of 1 for the epoch, to make the iteration below cleaner
    if normalize_by is None:
        normalize_by = tuple([1] + [1.0 for _ in range(n_data)])
    else:
        normalize_by = tuple([1] + list(normalize_by))

    data = np.loadtxt(stream, comments="#", dtype=dtype_dict)  # type: ignore

    output: list[PropertyData] = []
    zipped = zip(dtype_names, dtype_formats, normalize_by)
    next(zipped)  # first entry is for the epoch, which we don't need
    for name, dtype, norm in zipped:
        output.append(PropertyData(data[_EPOCH_NAME], np.array(data[name] / norm, dtype=dtype)))

    return output


def _create_dtype_names(n_data: int) -> tuple[str, ...]:
    other_column_names: list[str] = [f"value{i}" for i in range(n_data)]

    all_column_names = [_EPOCH_NAME] + other_column_names
    return tuple(all_column_names)


def _create_dtype_formats(n_data: int, dtypes: Optional[tuple[Type, ...]] = None) -> tuple[Type, ...]:
    if dtypes is None:
        return tuple([np.int32] + [np.float64 for _ in range(n_data)])
    else:
        return tuple([np.int32] + list(dtypes))
