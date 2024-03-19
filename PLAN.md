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

### PLAN
- [DONE:2024-03-16] finish code to read a histogram file, update the contents, and rewrite them
- [DONE:2024-03-16] implement the `g(r)`
- simplify the header interface by putting several headers into a single one (too complicated right now)
- [DONE:2024-03-18] implement `centroid g(r)`
- create python scripts to extract and display important information from the output files
  - this will require numpy, matplotlib, etc.
- make saving the histograms atomic
- implement continuing an interrupted simulation
- implement the 3BPES
- implement the 4BPES
