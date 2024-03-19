# pimc_simpy

This is a Python subproject of the C++ pimc-sim project.

Its current purpose is to create a package for all the code that makes is easier to analyze and plot the data collected from a simulation.

I decided to create it as a subproject of the C++ project because
- the code for analyzing the data is tied to the format of the output files from the C++ project, making them tightly coupled
- a user that only wants the C++ code, or only the Python code, isn't forced to compile/set up the other
- the project isn't at a size where the organization complexity of having a Python subproject is too large
