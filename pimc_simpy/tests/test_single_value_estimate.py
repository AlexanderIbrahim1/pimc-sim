from io import StringIO

import numpy as np
from numpy.testing import assert_array_almost_equal

from pimc_simpy.single_value_estimate import read_property_data_stream
from pimc_simpy.single_value_estimate import PropertyData


def test_read_property_data_stream() -> None:
    stream = StringIO(
        "\n".join(
            [
                "# this is a comment      ",
                "# this is another comment",
                "00005    1.23456         ",
                "00006    2.34567         ",
                "00007    3.45678         ",
            ]
        )
    )

    expected = PropertyData(
        np.array([5, 6, 7], dtype=np.int32), np.array([1.23456, 2.34567, 3.45678], dtype=np.float64)
    )
    actual = read_property_data_stream(stream)

    np.testing.assert_array_equal(expected.epochs, actual.epochs)
    np.testing.assert_array_almost_equal(expected.values, actual.values)
