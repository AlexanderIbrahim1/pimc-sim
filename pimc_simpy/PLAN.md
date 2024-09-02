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



## 2024-09-01

### What types of files are there so far?

1. Two column file of format (i_block, value)
  - `absolute_centroid_distance.dat`
  - `centre_of_mass_step_size.dat`
  - `kinetic.dat`
  - `pair_potential.dat`
  - `rms_centroid_distance.dat`

2. Three column file of format (i_block, value0, value1)
  - `bisection_multibead_position_move_accept.dat`
  - `bisection_multibead_position_move_info.dat`
  - `centre_of_mass_position_move_accept.dat`
  - `single_bead_position_move_accept.dat`

3. Four column file of format (i_block, value0, value1, value2)
  - `timer.dat`

4. Histograms
  - `centroid_radial_dist_histo.dat`
  - `radial_dist_histo.dat`


### Miscellaneous
1. The `box_sides.dat` file:
  - an integer, followed by the sides of the box (depends on the dimensionality)

2. `continue.toml`

3. `prng.state`

4. `worldlines00000.dat`


## What do I do for now?
I want to make it convenient to read the data from the files of 2-, 3-, and 4-column format
  - they contain information about the MC step sizes and the estimators

I already have code for reading the histograms
  - I just need some functions for the multicolumn data


## How to proceed with the code?
- [DONE] move the plotting functions into a separate subpackage
- [DONE] put the `single_value_estimate.py` file in the `data` subpackage
  - move out the functions about plotting and reading data
- [DONE] create a separate module with functions to read the multicolumn data
  - each useful column can be read as a separate property
- [DONE] create functions that perform slicing and searching by epoch index
  - but don't use operator overloading, because that would be very confusing for the user
  - instead, create a separate function to do this
  - in fact, maybe don't implement `__setitem__`, `__getitem__`, etc.
    - after all, `epochs` and `values` are publicly accessible, and they can work directly with those
  - the functions to implement should be called:
  ```
  element_by_index(index: int, property: PropertyData) -> np.float64
  element_by_epoch(index: int, property: PropertyData) -> np.float64
  slice_by_index(slice: slice, property: PropertyData) -> PropertyData
  slice_by_epoch(slice: slice, property: PropertyData) -> PropertyData
  ```
- right now, the property plotting code only shows the cumulative means and sems
  - what if I just want to see the property itself, without the cumulative means?
- what if I want two different PropertyData instances to shift their epochs?
  - maybe ignore the fact that the epochs are on different labels?
- create a plotting function that:
  - takes several PropertyData instances
  - normalizes all their values between 0 and 1
  - plots them all
  + the point wouldn't be to look at their values, but rather to see where they level out
    + so I can see the equilibration time
