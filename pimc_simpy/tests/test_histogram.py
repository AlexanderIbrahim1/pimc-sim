from io import StringIO

import numpy as np
import pytest

from pimc_simpy.histogram import HistogramInfo
from pimc_simpy.histogram import OutOfRangePolicy
from pimc_simpy.histogram import read_histogram_stream


def test_read_histogram() -> None:
    stream = StringIO(
        "\n".join(
            [
                "# This file contains the state of a regularly-spaced histogram              ",
                "# The layout for the histogram data is as follows:                          ",
                "# - [integer] the out-of-range policy (0 = DO_NOTHING, 1 = THROW)           ",
                "# - [integer] the number of bins                                            ",
                "# - [floating-point] the minimum value                                      ",
                "# - [floating-point] the maximum value                                      ",
                "# ... followed by the count in each histogram bin, in single-column order...",
                "0                                                                           ",
                "12                                                                          ",
                "0.00000000e+00                                                              ",
                "9.28088750e+00                                                              ",
                "0                                                                           ",
                "10                                                                          ",
                "15                                                                          ",
                "0                                                                           ",
                "123                                                                         ",
                "456                                                                         ",
                "789                                                                         ",
                "0                                                                           ",
                "0                                                                           ",
                "321                                                                         ",
                "654                                                                         ",
                "987                                                                         ",
            ]
        )
    )

    expected = HistogramInfo(
        OutOfRangePolicy.DO_NOTHING,
        12,
        0.00000000e00,
        9.28088750e00,
        np.array([0, 10, 15, 0, 123, 456, 789, 0, 0, 321, 654, 987]),
    )

    actual = read_histogram_stream(stream)

    assert actual.policy == expected.policy
    assert actual.n_bins == expected.n_bins
    assert actual.minimum == pytest.approx(expected.minimum)
    assert actual.maximum == pytest.approx(expected.maximum)
    assert all(actual.bins == expected.bins)
