"""
This script contains pseudocode to help me find out what I need to implement to be able
to run the jobs manager.
"""

from pathlib import Path

import numpy as np


def get_toml_file_contents(abs_output_dirpath: Path | str, abs_repo_dirpath: Path | str, density: float) -> str:
    contents = "\n".join(
        [
            f"abs_output_dirpath = '{str(abs_output_dirpath)}'",
            "first_block_index = 0",
            "last_block_index = 200",
            "n_equilibrium_blocks = 10",
            "n_passes = 2",
            "n_timeslices = 16",
            "centre_of_mass_step_size = 0.3",
            "bisection_level = 3",
            "bisection_ratio = 0.5",
            f"density = {density:.8f}",
            "temperature = 4.2",
            f"abs_two_body_filepath =   '{str(abs_repo_dirpath)}/potentials/fsh_potential_angstroms_wavenumbers.potext_sq'",
            f"abs_three_body_filepath = '{str(abs_repo_dirpath)}/playground/scripts/threebody_126_101_51.dat'",
            f"abs_four_body_filepath =  '{str(abs_repo_dirpath)}/playground/scripts/models/fourbodypara_8_16_16_8.pt'",
        ]
    )

    return contents


def get_simulation_dirpath(abs_sim_root_dirpath: Path | str, sim_id: int) -> Path:
    return Path(f"{str(abs_sim_root_dirpath)}", "playground", "many_simulations_test", f"simulation_{sim_id:0>3d}")


def get_simulation_output_dirpath(abs_sim_dirpath: Path | str) -> Path:
    return Path(abs_sim_dirpath) / "output"


def get_slurm_file_contents(
    abs_executable_filepath: Path | str,
    abs_toml_filepath: Path | str,
    abs_slurm_prefix: Path | str,
    sim_id: int,
    memory_gb: int,
) -> str:
    abs_slurm_output_filepath = f"{abs_slurm_prefix}-{sim_id:0>3d}-%j.out"
    contents = "\n".join(
        [
            "#!/bin/bash",
            "",
            "#SBATCH --account=rrg-pnroy",
            f"#SBATCH --mem={memory_gb}G",
            "#SBATCH --time=0-06:00:00",
            "#SBATCH --cpus-per-task=1",
            f"#SBATCH --output={abs_slurm_output_filepath}",
            "",
            f"{abs_executable_filepath}  {abs_toml_filepath}",
        ]
    )

    return contents


def get_slurm_filepath() -> str:
    pass


# def get_simulation_dirpath(abs_pimc_sim_dirname: Path | str, sim_id: int) -> str:
# def get_toml_contents(abs_sim_dirname: Path | str, abs_pimc_sim_dirname: Path | str, density: float) -> str:
# def get_slurm_file_contents(abs_executable_filepath: Path | str, abs_toml_filepath: Path | str, abs_slurm_prefix: Path | str, sim_id: int, memory_gb: int) -> str:


def example() -> None:
    n_densities = 31
    densities = np.linspace(0.024, 0.1, n_densities)  # ANG^{-3}

    abs_repo_dirpath = Path("/home/a68ibrah/research/simulations/pimc-sim")
    abs_executable_filepath = abs_repo_dirpath / "build" / "dev" / "source" / "pimc-sim"
    toml_filename = "sim.toml"  # okay to have it be the same for each simulation; makes commands simpler

    for sim_id, density in enumerate(densities):
        # create the locations for the simulation and the output
        abs_sim_dirpath = get_simulation_dirpath(abs_repo_dirpath, sim_id)
        abs_output_dirpath = get_simulation_output_dirpath(abs_sim_dirpath)

        abs_sim_dirpath.mkdir()
        abs_output_dirpath.mkdir()

        # create the toml file
        toml_contents = get_toml_file_contents(abs_output_dirpath, abs_repo_dirpath, density)
        toml_filepath = abs_sim_dirpath / toml_filename
        with open(toml_filepath, "w") as fout:
            fout.write(toml_contents)

        # create the slurm file (in a separate directory?)
        slurm_contents = get_slurm_file_contents(abs_executable_filepath)

    sim_directory_template = get_simulation_directory_template()  # type: ignore
    sim_dirpaths = [Path(sim_directory_template.format(i)) for i in range(n_densities)]

    toml_template = get_toml_template()  # type: ignore

    # the toml file needs to know about the simulation dirpath, otherwise
    toml_file_contents = [toml_template.apply(density, simpath) for density, simpath in zip(densities, sim_dirpaths)]

    # create the directories and copying the toml contents to them
    for simpath, toml_contents in zip(sim_dirpaths, toml_file_contents):
        simpath.mkdir(parents=True)

        toml_filepath = simpath / toml_filename
        with open(toml_filepath, "w") as fout:
            fout.write(toml_contents)

    # perform the simulations
    # - only need one version of the executable, so I don't need to copy it to each directory
    # - the paths in the toml file should be absolute, and unique to the simulation directory
    # - I need to perform the simulations through slurm

    # IDEA:
    # - use Python to create the bash files
    #   - the absolute path to the simulation directory is hardcoded into it
    #   - I can name each slurm file according to the conditions, and actually read their names
    #     properly in squeue
    # - based on previous simulations, the chances that I'll have to rerun a slurm file are pretty high
    #   - so it's not much of a hassle to have to create a separate file to run the bash files
    slurm_bash_template = get_slurm_bash_template()
    slurm_bash_contents = [slurm_bash_template.apply(simpath) for simpath in sim_dirpaths]

    for i, simpath in enumerate(sim_dirpaths):
        slurm_filepath = Path("slurm_files", f"coarse_{i}.sh")
        with open(slurm_filepath, "w") as fout:
            fout.write(slurm_bash_contents)


def main() -> None:
    toml_template = get_toml_template()

    replacement_dict = {
        "abs_sim_dirname": "/home/a68ibrah/research/simulations/pimc-sim/playground/ignore3",
        "abs_pimc_sim_dirname": "/home/a68ibrah/research/simulations/pimc-sim",
    }

    toml_file_contents = toml_template.format(**replacement_dict)
    print(toml_file_contents)


if __name__ == "__main__":
    main()
