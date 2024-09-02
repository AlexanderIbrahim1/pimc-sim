import pytest

import numpy as np
from numpy.typing import NDArray

from pimc_simpy.data.property_data import PropertyData
from pimc_simpy.data.property_data import between_epochs
from pimc_simpy.data.property_data import between_indices
from pimc_simpy.data.property_data import element_by_epoch
from pimc_simpy.data.property_data import element_by_index


@pytest.fixture
def basic_property_data_02468() -> PropertyData:
    epochs: NDArray[np.int32] = np.array([3, 4, 5, 6, 7], dtype=np.int32)
    values: NDArray[np.int32] = np.array([0, 2, 4, 6, 8], dtype=np.int32)

    return PropertyData(epochs, values)


class TestPropertyData:
    def test_element_by_index(self, basic_property_data_02468: PropertyData) -> None:
        assert element_by_index(0, basic_property_data_02468) == 0
        assert element_by_index(1, basic_property_data_02468) == 2
        assert element_by_index(2, basic_property_data_02468) == 4

    def test_element_by_epoch(self, basic_property_data_02468: PropertyData) -> None:
        assert element_by_epoch(3, basic_property_data_02468) == 0
        assert element_by_epoch(4, basic_property_data_02468) == 2
        assert element_by_epoch(5, basic_property_data_02468) == 4

    def test_between_epochs(self, basic_property_data_02468: PropertyData) -> None:
        expected = PropertyData(np.array([4, 5, 6], dtype=np.int32), np.array([2, 4, 6], dtype=np.int32))
        actual = between_epochs(4, 7, basic_property_data_02468)

        np.testing.assert_array_equal(actual.epochs, expected.epochs)
        np.testing.assert_array_equal(actual.values, expected.values)

    def test_slice_by_index(self, basic_property_data_02468: PropertyData) -> None:
        expected = PropertyData(np.array([4, 5, 6], dtype=np.int32), np.array([2, 4, 6], dtype=np.int32))
        actual = between_indices(1, 4, basic_property_data_02468)

        np.testing.assert_array_equal(actual.epochs, expected.epochs)
        np.testing.assert_array_equal(actual.values, expected.values)
