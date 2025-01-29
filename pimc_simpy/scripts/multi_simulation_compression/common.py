"""
This module contains functions that are used in the other scripts.
"""

from pathlib import Path


def get_abs_jobs_dirpath() -> Path:
    abs_local_repo_dirpath = Path("/home/a68ibrah/research/simulations/pimc-sim")
    rel_jobs_dirpath = Path("pimc_simpy/playground/twothreefour_body/pert2b3b4b/n180_pert2b3b4b_round1/simulations")

    return abs_local_repo_dirpath / rel_jobs_dirpath
