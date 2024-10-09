# Potential Energy Differences with old MORIBS code

These bugs have been fixed, but they are clogging the "PLAN.md" file, so I'm moving the notes here


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


### I tried running the estimators again, with a snapshot from the old code run on the local machine
The two-body energies match (almost, but close enough)
  - and I matched the centroid cutoff

The three-body energies do not match
  - I just realized that the old and new simulation code require three-body PES files of different formats
    - I changed all the local ones so that instead of having the coordinates for the R, s, cosu grid, they just have the endpoint limits
    - so I need a file that matches the old format


## I fixed the three-body potential issues!
The reason was that the calculation of `cosu_unclamped` had the incorrect sign
  - this flipped its sign to negative
  - so clamping it set the value of `cosu` to `0.0`
  - so all the interpolations kept hitting the incorrect energy

With the change, the old code (local) and new code give the same 3B energy

## The two-body energies are slightly off
I don't know why this is the case
  - I set the cutoff distances to the same thing (minimum side length of box / 2)
  - going between release and highperf doesn't change anything in the 2B, negligible change in 3B
    - and the old code was compiled on -O3

OLD CODE:
  2B ENERGY : -2.9305497158e+04 K -> -2.0368207034e+04 wvn
  3B ENERGY : -3.2526552306e+01 K -> -2.2606937801e+04 wvn

NEW CODE:
  2B ENERGY : -2.03312547e+04 wvn
  3B ENERGY : -2.26068183e+01 wvn

## The old and new PIMC codes evaluate different pairs even for the same snapshot
There are more terms that go into the sum in the old moribs code, than in the current pimcsim code


## FINALLY SOLVED
The reason was:
  - the old code was limiting the evaluation to between particles whose centres of mass were within the cutoff
  - the new code was limiting the evaluation to between individual beads within the cutoff
  - so the old code was rejecting more samples

I didn't change the current pimc-sim code, but I did check to make sure that this was the difference by changing the old code

And now both the 2B and 3B energies match!
