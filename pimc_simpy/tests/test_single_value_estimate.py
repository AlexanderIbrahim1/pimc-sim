from io import StringIO

import numpy as np

from pimc_simpy.data.property_data_reading import _read_property_data_stream
from pimc_simpy.data import PropertyData


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

    expected = PropertyData(np.array([5, 6, 7], dtype=np.int32), np.array([1.23456, 2.34567, 3.45678], dtype=np.float64))
    actual = _read_property_data_stream(stream, n_data=1)[0]

    np.testing.assert_array_equal(expected.epochs, actual.epochs)
    np.testing.assert_allclose(expected.values, actual.values)
