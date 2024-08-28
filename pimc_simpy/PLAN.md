# PLAN

## 2024-03-19
What kinds of data files are there, and what kinds of things do I want to visualize from them?

Is this stuff even worth doing?
- I assume more sophisticated analysis would require users to read data using the finer-grained numpy functions anyways
- however, certain things are looked at so frequently that we might as well create specialized functions for them
- I can just mention this in the project

### Histograms
- plot the histogram
- get the total number of samples
- get the other information stored in the histogram header
- create function to get the normalized g(r)
- separate histogram and radial distribution function; there are probably lots of types of histograms

### Files that store information per epoch
- move acceptance and rejection files
  - get the number of accepts, rejects, and percentage
  - get overall statistical data (mean percentage over all epochs, how much it varies)
  - plot this information as a function of epoch

- single value as a function of epoch
  - plot that information as a function of epoch
    - and also show the variance calculated up to that point (semi-transparent in the background)
  - get the mean, variance, other statistical data


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
