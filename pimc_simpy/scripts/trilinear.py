"""
This module contains an implementation of the Trilinear interpolator.
"""

import math
import numpy as np
from numpy.typing import NDArray

Limits = tuple[float, float]


class TrilinearInterpolator:
    def __init__(
        self, data: NDArray[np.float64], xlims: Limits, ylims: Limits, zlims: Limits
    ) -> None:
        self._data = data
        self._xlims = xlims
        self._ylims = ylims
        self._zlims = zlims
        self._dx = (xlims[1] - xlims[0]) / (self._data.shape[0] - 1)
        self._dy = (ylims[1] - ylims[0]) / (self._data.shape[1] - 1)
        self._dz = (zlims[1] - zlims[0]) / (self._data.shape[2] - 1)

    def __call__(self, x: float, y: float, z: float) -> float:
        self._check_is_in_limits("x", x, self._xlims)
        self._check_is_in_limits("y", y, self._ylims)
        self._check_is_in_limits("z", z, self._zlims)

        ix, iy, iz = self._lower_indices(x, y, z)

        xdiff = (x - (self._xlims[0] + ix * self._dx)) / self._dx
        ydiff = (y - (self._ylims[0] + iy * self._dy)) / self._dy
        zdiff = (z - (self._zlims[0] + iz * self._dz)) / self._dz

        xdiffm1 = 1.0 - xdiff
        ydiffm1 = 1.0 - ydiff
        zdiffm1 = 1.0 - zdiff

        # fmt: off
        e000 = self._data[ix    , iy    , iz    ]
        e001 = self._data[ix    , iy    , iz + 1]
        e010 = self._data[ix    , iy + 1, iz    ]
        e011 = self._data[ix    , iy + 1, iz + 1]
        e100 = self._data[ix + 1, iy    , iz    ]
        e101 = self._data[ix + 1, iy    , iz + 1]
        e110 = self._data[ix + 1, iy + 1, iz    ]
        e111 = self._data[ix + 1, iy + 1, iz + 1]
        # fmt: on

        f00 = xdiffm1 * e000 + xdiff * e100
        f01 = xdiffm1 * e001 + xdiff * e101
        f10 = xdiffm1 * e010 + xdiff * e110
        f11 = xdiffm1 * e011 + xdiff * e111

        g0 = ydiffm1 * f00 + ydiff * f10
        g1 = ydiffm1 * f01 + ydiff * f11

        return zdiffm1 * g0 + zdiff * g1

    def _lower_indices(self, x: float, y: float, z: float) -> tuple[int, int, int]:
        i_x_float = (x - self._xlims[0]) / self._dx
        i_y_float = (y - self._ylims[0]) / self._dy
        i_z_float = (z - self._zlims[0]) / self._dz

        i_x = math.floor(i_x_float)
        i_y = math.floor(i_y_float)
        i_z = math.floor(i_z_float)

        return i_x, i_y, i_z

    def _check_is_in_limits(self, name: str, value: float, limits: Limits) -> None:
        # NOTE: because we subtract the EPS
        if not (limits[0] <= value < limits[1]):
            raise ValueError(f"The value '{name} = {value}' is out of limits.")
