"""
Each job only performed the simulation for a single block, and thus only collected
a single block's worth of data for an estimate.

This script collects all the estimates for a given estimator, and collects them into
a single file. The block indices are replaced.
"""

from io import TextIOWrapper
from pathlib import Path

from block_index_mapping import read_block_to_job_index_map
from common import get_abs_jobs_dirpath

# PLAN
# - decide on the name of the file to collect data for
# - loop over all the block indices, and the corresponding job index
# - read in the header from one of the jobs
#   - use it in all the other files
# - read in the non-comment line (there should only be one)
#   - replace '00001' with the formatted version of the block index
#   - save the line in a list
# - create an output file with the same name as the name of the files data was collect from
#   - write the header
#   - write out all the lines
#   - WATCH OUT FOR NEWLINES


def _read_header(instream: TextIOWrapper) -> list[str]:
    comments: list[str] = []

    while True:
        pos = instream.tell()
        line = instream.readline()

        # there are no more lines
        if line == "":
            break

        # the line just read is not a comment
        if not line.startswith("#"):
            instream.seek(pos)
            break

        comments.append(line.strip())  # Add the comment line to the list without newline characters

    return comments


def _data_filepath(filename: str, i_job: int) -> Path:
    abs_jobs_dirpath = get_abs_jobs_dirpath()
    return abs_jobs_dirpath / f"job_{i_job:0>3d}" / "output" / filename


def compress_data(filename: str) -> None:
    block_to_job_index_map = read_block_to_job_index_map()

    arbitrary_data_filepath = _data_filepath(filename, 0)
    with open(arbitrary_data_filepath, "r") as instream:
        header = _read_header(instream)
        header = "\n".join(header) + "\n"

    data_lines: list[str] = []
    for i_block, i_job in block_to_job_index_map.items():
        data_filepath = _data_filepath(filename, i_job)

        with open(data_filepath, "r") as instream:
            # skip the header comments
            _read_header(instream)

            n_instances_to_replace = 1
            data_line = instream.readline()
            data_line = data_line.replace("00001", f"{i_block:0>5d}", n_instances_to_replace)
            data_lines.append(data_line)

    output_filepath = Path(".", "output", filename)
    with open(output_filepath, "w") as outstream:
        outstream.write(header)
        for data_line in data_lines:
            outstream.write(data_line)


if __name__ == "__main__":
    filenames = [
        "absolute_centroid_distance.dat",
        "bisection_multibead_position_move_accept.dat",
        "centre_of_mass_position_move_accept.dat",
        "kinetic.dat",
        "pair_potential.dat",
        "quadruplet_potential.dat",
        "rms_centroid_distance.dat",
        "single_bead_position_move_accept.dat",
        "timer.dat",
        "triplet_potential.dat",
    ]

#     for filename in filenames:
#         compress_data(filename)
