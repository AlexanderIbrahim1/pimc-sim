"""
Only about 980 of the 1000 jobs finished. The analysis scripts have trouble working
with block indices when certain ones are missing. To make it easier for the analysis
scripts, the job indices (between 000 and 999) will be mapped to artificial block
indices (between 000 and however many jobs completed).
"""

from pathlib import Path
from io import TextIOWrapper

from common import get_abs_jobs_dirpath

BLOCK_INDEX_FILEPATH: Path = Path(".", "block_to_job_index_map.txt")


def read_block_to_job_index_map() -> dict[int, int]:
    def read_job_and_block_index(line: str) -> tuple[int, int]:
        tokens = line.split()
        i_block = int(tokens[0])
        i_job = int(tokens[1])

        return i_block, i_job

    jobs_to_blocks: dict[int, int] = {}
    with open(BLOCK_INDEX_FILEPATH, "r") as instream:
        for line in instream:
            i_block, i_job = read_job_and_block_index(line)
            jobs_to_blocks[i_block] = i_job

    return jobs_to_blocks


def _get_abs_indicator_filepath(i_job: int, abs_jobs_dirpath: Path) -> Path:
    rel_output_dirpath = Path(f"job_{i_job:0>3d}", "output")
    indicator_filename = "quadruplet_potential.dat"

    return abs_jobs_dirpath / rel_output_dirpath / indicator_filename


def _write_block_index_map(outstream: TextIOWrapper, finished_jobs: list[int]) -> None:
    header = "# [job index]   [block index]"
    for i_block, i_job in enumerate(finished_jobs):
        line = f"{i_block:0>3d}   {i_job:0>3d}\n"
        outstream.write(line)


def main() -> None:
    abs_jobs_dirpath = get_abs_jobs_dirpath()

    n_jobs = 1000
    finished_jobs: list[int] = []

    for i in range(n_jobs):
        abs_indicator_filepath = _get_abs_indicator_filepath(i, abs_jobs_dirpath)
        if abs_indicator_filepath.exists():
            finished_jobs.append(i)

    with open(BLOCK_INDEX_FILEPATH, "w") as fout:
        _write_block_index_map(fout, finished_jobs)


if __name__ == "__main__":
    main()
    jobs_to_blocks = read_block_to_job_index_map()
    print(jobs_to_blocks)
