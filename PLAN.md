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
- [DONE:2024-04-02] implement the 3BPES
  - [DONE] finish creating the smaller three-body PES grid, and verify that it gives the expected energies for specific configurations
    - [DONE] do the verification in Python first, then some unit tests in C++
  - create estimators for the 3BPES
  - create unit tests for the ATM potential in specific geometries
- [DONE:2024-06-11] check if the Attard minimage condition is implemented properly in the 3BPES wrapper - this was the source of the error in the earlier simulation code!!!
- [DONE:2024-06-12] create interaction handler that allows both 2B and 3B interactions
- [DONE:2024-06-12] fix the three-body potential estimator
  - the energies are positive, and way too big (about 1/2 the magnitude of the pair energy!)
- [DONE:2024-07-26] implement the 4BPES
- [DONE:2024-07-26] come up with ideas for unit tests for the four-body PES
- [DONE:2024-08-10] solve a suspected bug with the dispersion potential
  - [NOT-TRUE] I suspect that there are underflow errors with the dispersion potential when using 32-bit floats
    - this is because we take a number to the power of 12
    - for example, for a distance of 5 Angstroms, we get `4.096e-09 ANG^{-12}`
  - [NOT-TRUE] I might be able to solve this by rescaling all the distances by the 12th root of the Bade coefficient
    before perform all the math operations on it
    - [DONE] I attempted this solution, and both the original and rescaled dispersion potentials give the same result
      even for a tetrahedron of side length 14.0f
- [DONE:2024-08-30] implement saving the PRNG state (lower priority? the physics is still the same I guess)
- [DONE:2024-08-??] modify main so it reads the arguments from an actual toml file instead of a string view
- [DONE:2024-08-30] make saving the histograms atomic
- [DONE:2024-08-31] implement a timer
  - so I know how long each pass takes, and how long a simulation is expected to run for
- [DONE:2024-08-31] allow worldlines and continue files to be created during equilibration
  - in case a simulation needs to be continued after being interrupted during equilibration
- remove the `playground` directories from the repo entirely
  - they should be in `.gitignore` and be a place to put files that don't clutter the repo
  - any important files in there should be moved to other directories
- consolidate the different `XValueBlockWriter` classes into a single class
  - they share so much functionality, it looks like I can clean up the code by a lot
- implement logging - simplify the header interface by putting several headers into a single one (too complicated right now)
- convert the project into a header-only library, since that's what it is basically becoming
  - this also makes it easier to turn the library into a formal target for cmake, and makes it
    less sketchy to include headers from external projects
- create separate executables
  - one for running the simulation *and* calling certain estimators
  - another for reading worldline files and calling certain estimators on them
  + this is because the 3B potential estimator is actually much slower than the 3B potential sampler
    + the time complexity makes a difference!
    + I might want to skip calling the estimator so I can go through more states in a simulation


### IDEAS (FOR FUTURE, AFTER THIS PUBLICATION)

I've got temporary workarounds for these issues that will work for the current project
  - but if I want this codebase to work for later projects, I need to fix these design issues

#### Making it clearer what the inputs for the different potentials are
Some of the potentials can take the distance as the argument, other take the distance squared
  - I want a way to make sure the user does not confuse one for the other!
  - otherwise, it could ruin their results!

I can *sort of* get away with this by making the distance measure part of the potential
  - so the potential always takes two points, and the details are internal to the potential
  - thus the user can never make the mistake

But there's probably a better design choice here

#### Making the InteractionHandler instances account for buffering
Notice that the `operator()` member functions for InteractionHandler instances only take the particle index and the worldine
  - the user doesn't need to care how the potential itself it called
    - it can call each one individually, or perform buffered calls, etc.

I want the user to not have to care about the specific interface of the interaction potential itself
  - (as long as it satisfies one of a few categories, otherwise they need to write their own)

I should:
  - create more concepts to cover more ways to call the interaction potential
  - modify the InteractionHandler instances with `constexpr if` to evaluate the potential
    differently, depending on which concept is satisfied

PROBLEM: mutability of buffered potentials
  - the non-buffered potentials do not need to mutate for any of their member functions to work
    - their state does not change after creation
  - the buffered potentials *DO* hold state
    - they need to store all their inputs so they can pass them all at once!
  - this messes with the `const`ness of a lot of other operations

Should I just make them non-const?
Should I separate the buffer from the buffered potential?

#### Making a Worldlines interface
Right now, a worldline is just a vector of `Cartesian<FP, NDIM>` instance
  - and the entire system state is given in a `std::vector<Worldline<FP, NDIM>>` instance

But a better idea might be to create a `Worldlines` interface that other concrete types can implement
  - it will be agnostic to the underlying data layout
  - this means different users can pick from different data layouts depending on what they need

Some data layouts are better than others for performance
  - and some data layouts are flat out impossible if more than one type of particle exists


### PUBLICATION PLAN
I want to starting running simulations to get results for the (2 + 3 + 4)-body simulations

- [DONE:2024-08-26] get the project working with real TOML files instead of reading from a string
  - because I need to modify the settings for each type of simulation
- [DONE:2024-08-27] I might want to make the paths to the PESs absolute, and read from the toml file
  - so the simulations can work regardless of the directory they are created in
- [DONE:2024-08-27] get the project compiling on the correct clusters
  - the ones that I have rgg priority on
  - NOTE:
    - Narval is *VERY* slow with the four-body PES, probably because of its use of AMD
    - Cedar is much faster
- [SKIP:2024-08-28] maybe make the code that manages all the simulations, part of the same repo for the publication
  - the data analysis stuff is already there anyways
  - there are pros and cons (more localized stuff, mixing things that *might* be better off separate)
    - but I might as well try
      - this project is small enough that the consequences won't be too dire
      - I'll learn whether or not this is a good idea for future simulations
  - thought about it more, this is a bad idea
    - having to manage the simulations from a repo centred on writing tex files is strange
  - we are better off making the simulation management part of `pimc-simpy`
- come up with a way to manage all the simulations with all the different conditions
  - look up how I did it before
    - figure out the shortcomings, benefits, etc.
  - I can use the same strategy with the old `qmc.input` file with the new `.toml` file
- [DONE:2024-09-04] make sure I have all the estimators that I want (simulations are expensive, don't want to miss important things)
  - look at the older publications, and see what estimators I had back them
- [DONE:2024-09-04] create another executable that only evaluates worldlines
- modify the worldline evaluating executable so that:
  - it can evaluate multiple different worldines
  - and make sure the 4B estimator works too
- look at what other estimators I might be able to create


- there are some easy attempts at improving the performance we could try:
  - switch the default ThreeBodyParaH2Potential with the early rejector potential
  - switch the four-body neural network from SSP-64-128-128-64 to ReLU-8-16-16-8

- don't forget to run the final simulations in release mode

The simulations I want to run?
  - set `N = 180, P = 64` with a coarse-grained spread over the usual density range

The initial simulations?
  - FIRST, perform a few simulations at very different densities (low, medium, high)
    - make the simulation box smaller than usual, so the simulations are done more quickly
    - put these into a separate "trial simulations" directory
    - find the equilibration time
    - find the ideal MC move sizes
    - find the autocorrelation times
    - rerun again to see if the equilibration and autocorrelation times are the same if you
      fix the MC move sizes to their ideal values from the beginning, instead of letting them
      adjust during the equilibration phase
  - perform simulations under conditions that:
    - give OKAY results, at least good enough to know that the project works at all
      - maybe a reduced number of beads, a smaller potential?
    - can be performed very quickly, so I don't have to wait several days/weeks for results
    - can be *analyzed* very quickly (estimators take a lot of time too)

#### Finding equilibration time, ideal MC moves, and autocorrelation times
1. FIRST SIMULATION: the first thing we do is find the ideal MC moves
  - make the number of passes large enough for enough moves to accumulate
    - otherwise the MC move updates might be made using too few moves as data
  - make the number of equilibration blocks large enough for the move sizes to converge
  + the equilibration times we find here aren't useful anymore
    - once we fix the MC sizes to their ideal values, later simulations will have different equlibration times
  + we aren't concerned with autocorrelation times here
  + INHERENT ASSUMPTION: we perform enough MC moves to both converge the MC step sizes and reach equilibrium
2. SECOND SIMULATION: the next thing we do is find the new equilibration times, and autocorrelation times
  - fix the MC sizes to whatever we get in step 1
  - set the number of passes to 1 (so 1 pass = 1 block)
  - set the number of equilibration blocks to 0
  - wait until the estimator converges, and collect even more data
  + we can find the number of equilibration passes
  + we can find the number of passes needed for autocorrelation

Now:
  - we have the MC moves
  - we know how many passes are needed for autocorrelation
    - so define that number of passes as a block
  - we know how many passes are needed for equilbration
    - calculate the number of equilibration blocks based on the number of passes in a block

## Using smaller lattices to calculate MC info
I used the ideal step size I found for the 3 x 2 x 2 matrix, for the 5 x 3 x 3 matrix
  - they remained the same
  - this indicates that I can (probably) get away with using the smaller lattices to find the ideal MC steps

I got an autocorrelation time of 10 passes for the 3 x 2 x 2 matrix, for (P = 64, density=0.026)

### [INCORRECT] At density = 0.1
At this density, the COM and multilevel moves are frozen!
  - COM step size drops to 0
  - multilevel moves reduce to single bead moves
  - single bead moves have a success rate of ~10%

As a result, the autocorrelation time increases to ~150 passes (box = 3x2x2, P = 64, density = 0.1)
  - the only recourse here is:
    - pick a larger value of P so larger multilevel moves are allowed
    - remove the COM and multilevel moves, because they fail anyways
    - increase the number of passes for the single bead moves

[INCORRECT] it turns out this was a bug where I was undercounting the number of accepts
  - not sure how it changes the optimal step sizes, but they should not freeze anymore


## 2024-08-28

### Planning large-scale simulation jobs
There are times where I want to create lots of simulation jobs
  - each of them will run under a different set of conditions
    - although some of them might be duplicates!
  - I need this procedure to happen in an organized fashion

#### EXAMPLES
1. I want to run lots of simulations for the coarse-grained grid of densities
  - each simulation has:
    - a different density
    - different ideal monte carlo steps, autocorrelation times, equilbration times
    - the same number of beads, number of particles, types of potentials, etc.

2. I want to run simulations for the fine-grained grid of densities around the equilibrium density
  - each simulation has:
    - a different `(density, n_beads)` combination
    - different ideal monte carlo steps, autocorrelation times, equilbration times
    - the same number of particles, types of potentials, etc.

#### What do I need?
It looks like I need a way to:
  - create ways to label the different simulations that I want
    - including creating a file that describes what is going on in a given directory
  - create the appropriate directories, one for each simulation
  - create the corresponding toml files
  - run the jobs via slurm (don't need to abstract this just yet)


## Memory Requirements on Clusters

### Some back of the envelope calculations
With (N = 180, P = 64), a single wordline file is about 588 kB
  - extending to (N = 180, P = 960), this will go up to about 7 MB
  - if I have 300 of those, I'll end up with about 2 GB
  - with 31 densities, this will be about 60 GB

So the clusters should have enough memory on them
  - IIRC, the variance of the 4B total energy is much lower than for the 2B and 3B total energies
  - so I can use a random subset of the saved worldlines for the total 4B energy calculations

## BUG: figure out why sign of the 3B total energy is flipped now!
I remember the 3B total energy being negative at the equilibrium density
  - but now it is positive
  - it doesn't become negative until a slightly higher density of `rho = 0.03` (rather than `rho = 0.026`)
  - the `g(r)` definitely says it spends a lot of time in the negative energy region!

Things I have tried, which have not worked:
  1. running the simulation using double instead of float; DID NOT CHANGE ANYTHING
  2. changing the periodicity from Attard to the (incorrect) regular minimage
    - maybe the fact that it was negative in the old paper was the result of that bug
    - but this IS NOT THE CASE!
    - it actually gets MORE POSITIVE:
    ```[attard periodicity]
    # total triplet potential energy in wavenumbers
    00060   2.94176636e+02
    00061   2.73421997e+02
    00062   2.88672577e+02
    00063   3.05167999e+02
    00064   3.01176056e+02
    00065   2.95887482e+02
    ```
    ```[incorrect periodicity]
    # total triplet potential energy in wavenumbers
    00060   4.00757660e+02
    00061   3.80848083e+02
    00062   3.95636566e+02
    00063   4.11265961e+02
    ```
  3. switching between `dev` and `dev-highperf`
    - same output
  4. constantly updating the nearest neighbours grid does not work (although it fixed a bug)
  5. going from NearestNeighbour to Full for the pair sampling


Some more things I could try:
  - roll back to an older version of the repo, and see if I can get the 3B PES to output negative energies
    at the equilibrium density again

I looked at the three-body energies collected for the original 2B-3B-4B perturbative EOS
  - and they were slightly positive!!!

I looked at the only MC code
  - there *was* a part of it that did early rejections of certain triangles, but it wasn't used for the estimator
    - so I really doubt that rejecting too many large triangles in the old simulation code is the reason it was slightly negative
  - and I just tried using the incorrect periodicity in this simulation, and the energies were even *MORE* positive

[WRONG:NOT_FOUND] I *MIGHT* have found the reason (a rescaling bug in the original code):
  - the old code used energies in units of Kelvin instead of wavenumbers
  - but the input file was always in units of wavenumbers
  - so when I read in the energies, I used a function to rescale all the energies in the grid
    from units of wavenumbers to units of kelvin
  - BUT I didn't do so for the ATM potential
    - that stayed in units of wavenumbers (which are smaller numerically compared to Kelvin)
  + to test this, I could run the estimators with the ATM potential:
    1. turned off -> DID NOT WORK! STILL POSITIVE!!!
    2. rescaled down by "kelvin per wavenumber" -> DID NOT WORK! STILL POSITIVE!!!
  + this means the possibility that early rejection above caused the bug is also incorrect
    - if turning off the ATM potential completely did nothing, then ignoring certain configurations wouldn't do anything either,
      even if it did happen during estimation

[WRONG] It is not the definition of the box sides for the periodic lattice
  - both the old code and the new code use the full box length, not the half box length
```Attard paper
The situation can be resolved generally by defining the `x` component of the lattice translation vectors,

t_{ij} = [(x_i - x_j) / L] * L
t_{ik} = [(x_i - x_k) / L] * L

where l (typo, should be uppercase?) is the box length, and...
...
The cutoff convention sets the triplet potential to zero if any side of the triangle
has a length greater than L/2.
```

[WRONG] I fixed a bug in the creation of the HCP lattice
  - changed the x-values for two of the basis cell points from 0.0 to 0.5
  - I also rederived the lattice positions on paper, and checked against the old moribs and the old python lattice repo
    - so I'm pretty sure it's correct now
  - it didn't fix the discrepancy between the old and new 3B energies

It looks like the old PIMC code *DID* properly implement the 3-body attard minimage rules?
  - so what exactly was the issue?
  - or maybe that's a version that I updated, and I compiled with an older version
    - unfortunately I didn't use version control at the time so I can't tell for certain

## [DONE] Fix bug in the current 3-body code
There is a bug in the current 3-body code
  - it *MIGHT* be responsible for why the three-body energy is slightly positive, but it's definitely there
  - here's the output of the three-body simulation with P = 960 beads:
  ```triplet_potential.dat
  # total triplet potential energy in wavenumbers
  00020   2.02011871e+02
  00021   -9.63872250e+05
  00022   1.87773575e+02
  00023   -3.53046875e+06
  ```
  - I'm not sure what's causing the erratic results?
    - equililbrium *SHOULD* have been reached here already
    - I used the same MC move steps written in the graham code

Take the buggy worldline snapshot
  - load it into the code
  - print out the energies for every timeslice
  - find out which timeslice(s) give the erratic behaviour
  - look at the three-body energies for every single triplet
    - find out what is happening with that triplet

The issue occured when the shortest pair distance was exactly 5.5 angstroms
  - the control flow would cause the grid interpolation to be called
  - and 5.5 angstroms is exactly at the cusp where the index falls outside the grid

I fixed it by switching the control flow order


## Finding the difference between the old and new three-body energies
1. Convert a worldline from the old file format into one with the new file format, and calculate the energy here
  - check if the energies match
  - if the energy is negative, then the problem is with the sampling
  - if the energy is still positive, then the difference is the potential

2. Check if the two-body energies in the old and new code match
  - if they match, the difference is the three-body potential
  - if they differ, the difference is either the two-body potential, or the sampling

### The two-body energies do not match
I took a worldline file from the old moribs code
  - density = 0.0251 ANG^{-3}, P = 64, N = 180

I reformatted the worldline file to fit the new simulation code format
  - then I recalculated the 2B and 3B energies, with the estimators only

NEW CODE
  - total 2B potential: -1.94617500e+04 wvn
  - total 3B potential:  3.05606598e+02 wvn
OLD CODE
  - total 2B potential: -2.8640198083e+04 K = -1.99058041e+04 wvn
  - total 3B potential: -6.6980686797e+01 K = -4.65536035e+01 wvn

### I recompiled the old simulation code, and got simulations running on it
[NOTE:NOT_THE_REASON] 1. The new code's 2B energy is slightly less negative than the old code's 2B energy
  - when simulating at rho = 0.026 ANG^{-3}, P = 64, N = 180

The old code only includes energies if the centroids are within a certain distance
  - the new code includes energies if the beads themselves are within a certain distance
  - check if this is responsible for the discrepancy

This probably isn't the reason for the discrepancy
  - if it were, then the new code would give a more negative 2B energy than the old code
  - because the old code is rejecting more long-range (attractive) terms

[2024-05-10]:
  - implemented this, and I was right
    - the 2B energy became slightly less negative
    - the 3B energy became slightly less positive

2. The old code and new code have different 3B energies
  - still not sure why this is the case

3. The energies from the pimcanalysis on graham are negative!
  - I want to try and run the unoptimized three-body estimator
    - maybe the "fancy, faster" estimator has a bug in it?
  - right now I'm dealing with an issue where I can't run the code because
    of an incorrect version of numpy?
  - it's too complicated to fix!

4. I tried running the new PIMC code with no minimage
  - but I haven't tried it with *incorrect* minimage
  - and these will give different results
    - incorrect minimage will give fictitious smaller triangles!
  - so this is something to try!

5. Maybe the reason is the old code was running with units of Kelvin instead of wavenumbers
  - this might make a difference when interpolation is involved?
  - although I doubt it, but it should be fast to check

6. The old simulation code is now giving positive energies for the 3B PES (rho=0.0251, P=64, N=180)
  - the simulations for the perturbative EOS on graham did not run the 3B PES at all
  - instead, that was handled by the pimcanalysis jobs
  - so I should check those out

But then why did an older simulation on graham give negative energies too?
  - I hope I'm not getting different results on graham vs local
  - but the 2B energies from the (old code, graham) and (old code, local) match
    - so that's a good sign

7. Just fixed a memory error for the 3B potential (althouh it looked like it didn't matter to begin with)
  - just now, I found a bug where I reserved memory for the 3B energy grid, but I used `>>` to
    fill in the grid from the stream, rather than using push_back
  - I fixed this with a resize
  - it didn't change the energy on a local simulation

8. IDEA: load the new pimc-sim code onto graham, run a calculation there
  - maybe it's the server that makes the difference?

### Different optimization settings give different energies with 2B, but not 3B?
Applying the estimators on the moribs 370 worldlines (p=64, rho=0026, N=180)
  - the effect of optimization on the result is much smaller with double than with float

#### FP = float
2-BODY:
DEV:      00370   -1.94464746e+04
HIGHPERF: 00370   -1.94617500e+04

3-BODY:
DEV:      00370   3.05606598e+02
HIGHPERF: 00370   3.05606598e+02

#### FP = double
2-BODY:
DEV:      00370   -1.94419184e+04
HIGHPERF: 00370   -1.94419184e+04

3-BODY:
DEV:      00370   3.05606007e+02
HIGHPERF: 00370   3.05605997e+02
