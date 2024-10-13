# Renaming Worldlines

DONE: [2024-10-13]

Apparently I've been using the incorrect terminology the entire time
  - a worldline is the ordered set of beads that all correspond to the same particle
  - I've been using it to refer to the ordered set of beads that all correspond to the same timeslice,
    for all the different particles

I need to call them something else

### Some names, and the pros and cons
replicas
  - apparently there's something called the "replica trick" in the context 

paths
  - too ambiguous
  - I want something that makes it more obvious that it refers too all particles with the same timeslice index

classical_image
  - better, but a bit wordy?

timeslice
  - lecture notes from Ceperley actually mention it specifically


## Changing the Worldline interface
Right now, the (incorrectly named) "worldline" is a vector of points all at the same timeslice

But I notice that:
  - most of the time I pass a vector of worldlines
  - it looks like the effects of other algorithms (like worm) make it so that it might not make
    complete sense to refer to entirely separate particles

So maybe I should just create a single "Worldlines" object, and pass it around
  - and keep any information about the data layout encapsulated

Right now, don't worry about extending the program to other particles with different data layouts
  - I'll leave that for when I actually need it
  - although it looks like many of the MC moves so far will only work on point particles, and not rotors
