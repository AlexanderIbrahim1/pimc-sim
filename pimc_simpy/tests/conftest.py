import os
from pathlib import Path

import pytest


@pytest.fixture(scope="session")
def abspath_to_tests():
    """
    Certain tests depend on reading the contents of files. These files are stored
    in subdirectories within the `tests` directory. We need the test functions to
    know the absolute path to the `tests` directory, so that these test functions
    can always find the required files.

    Calling `Path.cwd()` and `os.getcwd()` both fail for whatever reason; I think
    it has something to do with how 'conftest.py' as a file is specifically handled.
    We use a hacky workaround involving `__file__`, `os` and `pathlib.Path`.
    """
    name_of_abspath = os.path.dirname(os.path.abspath(__file__))
    return Path(name_of_abspath)
