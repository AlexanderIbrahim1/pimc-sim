# Fixing MC move acceptance rate

DONE: [forgot the date]

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

