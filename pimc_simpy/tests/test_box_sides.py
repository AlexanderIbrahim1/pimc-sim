from io import StringIO

import pytest

from pimc_simpy.data.box_sides.box_sides import BoxSides
from pimc_simpy.data.box_sides.box_sides import read_box_sides_stream


class TestBoxSides:
    def test_reading(self) -> None:
        # contents are taken from an example simulation; the comments might change, but it is
        # unlikely that the values will
        contents = "\n".join(
            [
                "# this file contains information about the sides of the periodic box used in the simulation               ",
                "# the first uncommented line contains the number of dimensions                                            ",
                "# the following lines contain the side lengths, in order of the axis they belong to                       ",
                "# for example, in 3D there would be 4 lines:                                                              ",
                "# the first has the integer 3, and the next three are the x-axis, y-axis, and z-axis lengths, respectively",
                "3                                                                                                         ",
                "1.16740761e+01                                                                                            ",
                "1.34800615e+01                                                                                            ",
                "1.27091236e+01                                                                                            ",
            ]
        )

        stream = StringIO(contents)

        actual = read_box_sides_stream(stream)
        expected = BoxSides(1.16740761e01, 1.34800615e01, 1.27091236e01)

        assert actual.n_dimensions == expected.n_dimensions
        assert actual[0] == pytest.approx(expected[0])
        assert actual[1] == pytest.approx(expected[1])
        assert actual[2] == pytest.approx(expected[2])
