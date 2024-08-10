# Plan for PIMC-SIM

## Goal
The goal of this project is to simulate solid parahydrogen with many-body interaction potentials

- I should be able to create pointwise molecules
  - but it should be flexible enough to create more molecules in general
- I should suppose distance functions both with and without periodicity
- I should have separate sections of code for sampling and estimation
- I should have a way to collect information from a TOML file, to support simulations with different parameters


## Particles
The simulation's performance is mostly bound by its ability to calculate interaction potentials
- interaction potentials are only calculated between particles on the same worldline
- this implies that, for cache optimization, the beads should be worldline-contiguous, not particle-contiguous

### Worldline-contiguity
+ better cache layout (points are closer to each other in memory)
+ probably easier to go from the classical case (1 bead) to the quantum case (many beads)
  + because the classical case is just a single worldline

- code becomes more complicated if there is more than a single variety of particle

### Particle-contiguity
+ simpler to implement in general
+ more intuitive to think about

- worse cache locality, worse performance for large systems


## PRNG
I need a random number generator that I can pass into functions
- I can use the Mersenne Twister for now, maybe switch to a faster one in the future


## Interaction Handlers
I need some sort of function that deals with calculating interactions between particles in the system
There could be several; I'll need a different one for different MC moves

For example:
- consider an object that calculates the interaction potential energy experienced by a single bead
  - it holds all the different interaction potentials
  - it will accept the index of the bead of interest, and all the worldlines
  - it will calculate the interaction potential experienced by just that bead


## Potential Wrappers
Carrying a potential (that requires a pair distance) and different measure functions is annoying
- i.e. might need no box, might need a box for periodicity, don't know which potentials are used for which, etc.

To make this nicer, we could create classes that wrap together:
- a potential (that depends on pair distances, or possibly other stuff)
- the functions that take cartesian points, and calculate the stuff needed to call the potential


### 2024-02-25
- split interaction handler concept into its own file

- create a new handler that owns the adjacency matrix
  - it should have a function to update the adjacency matrix
    - takes worldlines
    - in the future, it might have to be extended to handle different cases
      for different particle types, so I don't want it "too embedded" in the handler
    - maybe the function takes another function that takes the particle indices and
      returns the cutoff distance for those indices?
      - I don't need this yet
    - problem: how do I choose the pair distance calculating function?
      - I don't want it to need to know about the periodic box (it might not exist)
  - it should only account for interactions involving those in its adjacency matrix


### 2024-03-19
- how do I set up the Python scripts?
  - create a completely separate repo?
  - [BETTER] create a separate Python package within this repo?
- the latter is probably the better option; more cohesive, easier for users
- after all, if a user only wants the Python, or only the C++, they don't have to run/compile the other
  - and the project's not at the stage where pulling in stuff that people don't use is too big of a hassle

- I'll try to find repos that have both a cmake-based C++ project, and a Python-based package system, in the same repo
  - and see what they do

- I decided to create a separate subproject `pimc_simpy` that will contain the entire python package there
  - this provides a clean separation between the C++ and Python parts of the project

### 2024-03-20
- how do I implement continuing an interrupted simulation?

#### Using a continue file to hold the most recently completed block index
[DONE:2024-03-20]
- in the previous MoRiBS, I had a qmc.continue file that I read the most recent block number from
- I assume I'll use the same strategy:
  - have a function that looks for a "continue" file
    - if I find it, then I'll do things that assume I'm continuing an interrupted simulation
    - if I don't find it, then I'll do things that assume I'm starting a brand new simulation

- what information should I store in it?
  - the most recently saved block index!
    - and this should be updated at the end of saving all the values; otherwise, I might lose data if
      the simulation is updated during the saving of the data
  - actually, that's probably it
    - everything else can be found from the written simulation code
  - NO! I also need the current state of the PRNG!
    - but that's a bit too much for now

#### Single and Double value writers need to change how they work
[DONE:2024-03-20]
- there's currently an issue with the writers
  - they create a brand new output file to write to upon construction
  - I would have to pass some sort of "continue simulation" flag into EVERY writer to prevent it
    from overwriting the existing output file when continuing a simulation!
  - SOLUTION: it's probably better to delay creating the output files until the first call to `.write()`
    - inside, it can check if the file exists at the start of each write
    - this is almost certainly not a performance-intensive part of the code, so this isn't an issue

- as far as the single/double value writers are concerned, all they need to know is the most recent
  block ID to properly update the results

#### Histograms and the worldlines need to be restored at the start of the simulation
[DONE:2024-03-21]
- the histograms need to be read at the start of the simulation
- the worldlines need to have their state restored

#### I need to create a function to read in the worldlines
[DONE:2024-03-21]
- right now, the sides of the box are part of the file; but they don't really need to be?
  - I should separate them from the rest of the worldline state, and put them into their own file

[DONE:2024-03-21]
- the benefits of splitting up the box sides from the rest of the worldline:
  - I can reuse the same worldline writer for a simulation whether or not it is periodic
  - I can store the box size in a single place

### PLAN
- [DONE:2024-03-16] finish code to read a histogram file, update the contents, and rewrite them
- [DONE:2024-03-16] implement the `g(r)`
- [DONE:2024-03-18] implement `centroid g(r)`
- [DONE:2024-03-20] create python scripts to extract and display important information from the output files
  - this will require numpy, matplotlib, etc.
- [DONE:2024-03-20] implement continuing an interrupted simulation
- [DONE:2024-03-21] separate writing the box sides, from writing the worldline
- [DONE:2024-03-21] implement a worldline reader
- [DONE:2024-03-21] give worldline a constructor from `std::vector`, and remove the `initializer_list` one
  - I want to be able to move the vector of coordinates into the worldline
- [DONE:2024-03-21] introduce "saving schemes" for the worldline writer, in case I don't want to save literally every single worldline
  - maybe I only want the most recent n worldlines to be saved, so some might have to be deleted?
  - NOTE: did not implement schemes; just a function to delete the worldline file that is n blocks behind the current one
    - this is much simpler
- [IN-PROGRESS:2024-04-02] implement the 3BPES
  - [DONE] finish creating the smaller three-body PES grid, and verify that it gives the expected energies for specific configurations
    - [DONE] do the verification in Python first, then some unit tests in C++
  - create estimators for the 3BPES
  - create unit tests for the ATM potential in specific geometries
- [DONE:2024-06-11] check if the Attard minimage condition is implemented properly in the 3BPES wrapper - this was the source of the error in the earlier simulation code!!!
- [DONE:2024-06-12] create interaction handler that allows both 2B and 3B interactions
- [DONE:2024-06-12] fix the three-body potential estimator
  - the energies are positive, and way too big (about 1/2 the magnitude of the pair energy!)
- make saving the histograms atomic
- implement the 4BPES
- implement logging
- implement saving the PRNG state (lower priority? the physics is still the same I guess)
- simplify the header interface by putting several headers into a single one (too complicated right now)
- convert the project into a header-only library, since that's what it is basically becoming
  - this also makes it easier to turn the library into a formal target for cmake, and makes it
    less sketchy to include headers from external projects
- modify main so it reads the arguments from an actual toml file instead of a string view
- create separate executables
  - one for running the simulation *and* calling certain estimators
  - another for reading worldline files and calling certain estimators on them
  + this is because the 3B potential estimator is actually much slower than the 3B potential sampler
    + the time complexity makes a difference!
    + I might want to skip calling the estimator so I can go through more states in a simulation
- come up with ideas for unit tests for the four-body PES
- [IN-PROGRESS:2024-08-10]: solve a suspected bug with the dispersion potential
  - [NOT-TRUE] I suspect that there are underflow errors with the dispersion potential when using 32-bit floats
    - this is because we take a number to the power of 12
    - for example, for a distance of 5 Angstroms, we get `4.096e-09 ANG^{-12}`
  - [NOT-TRUE] I might be able to solve this by rescaling all the distances by the 12th root of the Bade coefficient
    before perform all the math operations on it
    - [DONE] I attempted this solution, and both the original and rescaled dispersion potentials give the same result
      even for a tetrahedron of side length 14.0f
