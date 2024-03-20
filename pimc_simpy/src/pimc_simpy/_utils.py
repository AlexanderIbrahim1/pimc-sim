"""
This module contains functions used elsewhere in the package.
"""

from typing import TextIO


def skip_lines_that_start_with(fin: TextIO, start: str) -> None:
    while True:
        position = fin.tell()
        line = fin.readline().strip()

        if line == "":
            break

        if line.startswith(start):
            continue
        else:
            fin.seek(position)
            break
