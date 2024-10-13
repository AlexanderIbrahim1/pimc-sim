## Memory Requirements on Clusters

### Some back of the envelope calculations
With (N = 180, P = 64), a single wordline file is about 588 kB
  - extending to (N = 180, P = 960), this will go up to about 7 MB
  - if I have 300 of those, I'll end up with about 2 GB
  - with 31 densities, this will be about 60 GB

So the clusters should have enough memory on them
  - IIRC, the variance of the 4B total energy is much lower than for the 2B and 3B total energies
  - so I can use a random subset of the saved worldlines for the total 4B energy calculations
