"""
This script contains pseudocode to help me find out what I need to implement to be able
to run the jobs manager.
"""

from pathlib import Path

import numpy as np

from pimc_simpy.manage.replacement_template import ReplacementTemplate


def get_toml_template():
    contents = "\n".join(
        [
            "abs_output_dirpath = '[[abs_sim_dirname]]/output'",
            "first_block_index = 0",
            "last_block_index = 200",
            "n_equilibrium_blocks = 10",
            "n_passes = 2",
            "n_timeslices = 16",
            "centre_of_mass_step_size = 0.3",
            "bisection_level = 3",
            "bisection_ratio = 0.5",
            "density = 0.026",
            "temperature = 4.2",
            "abs_two_body_filepath = '[[abs_pimc_sim_dirname]]/potentials/fsh_potential_angstroms_wavenumbers.potext_sq'",
            "abs_three_body_filepath = '[[abs_pimc_sim_dirname]]/playground/scripts/threebody_126_101_51.dat'",
            "abs_four_body_filepath = '[[abs_pimc_sim_dirname]]/playground/scripts/models/fourbodypara_8_16_16_8.pt'",
        ]
    )

    return ReplacementTemplate(contents)


def get_simulation_directory_template():
    pass


def get_slurm_bash_template():
    pass


def example() -> None:
    n_densities = 31
    densities = np.linspace(0.024, 0.1, n_densities)  # ANG^{-3}

    sim_directory_template = get_simulation_directory_template()  # type: ignore
    sim_dirpaths = [Path(sim_directory_template.format(i)) for i in range(n_densities)]

    toml_filename = "sim.toml"  # okay to have it be the same for each simulation; makes commands simpler
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
        "[[abs_sim_dirname]]": "/home/a68ibrah/research/simulations/pimc-sim/playground/ignore3",
        "[[abs_pimc_sim_dirname]]": "/home/a68ibrah/research/simulations/pimc-sim",
    }

    toml_file_contents = toml_template.replace(replacement_dict)
    print(toml_file_contents)


if __name__ == "__main__":
    main()
