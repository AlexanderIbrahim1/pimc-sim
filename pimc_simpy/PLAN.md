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

