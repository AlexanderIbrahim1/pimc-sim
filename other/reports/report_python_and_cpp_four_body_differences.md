## 2024-08-10
I am porting the model from Python to C++
  - a few of the samples give different energies (maybe by several percent)

The problem is not floating-point underflow in the C++ dispersion potential
  - I have already checked
  - the original method and the rescaling method both give the same results, for 32-bit floats

The problem is (maybe) not the JIT tracing of the model from Python to C++
  - I put it in eval mode, and moved the model to the CPU before performing the trace
  - the energies that come out of the model are the same
  - loading the model using `torch.jit.load()` instead of `torch.load()` does not change the energies

I have noticed that the torch module sometimes gives a different output for the same input
Here is an example:
```cxx
// before rescale: energy[374] = 3.05985
// after rescale:  energy[374] = 0.0884702

batch_sidelengths[374] =  2.2500
 5.6115
 5.7409
 4.7545
 5.7409
 2.4504
[ CPUFloatType{6} ];
```

```py
# before rescale: energy[374] = 2.451932191848755
# after rescale:  energy[374] = 0.07315619936723838

batch_sidelengths[374] = tensor([2.2500, 5.6115, 5.7409, 4.7545, 5.7409, 2.4504])
```

Here is another example:
```cxx
// before rescale: energy[12] = -0.423913
// after rescale:  energy[12] = 0.000527995
// before rescale: energy[13] = -0.420834
// after rescale:  energy[13] = 0.000534421

batch_sidelengths[12] =  2.2500
 4.4444
 4.6358
 5.5642
 5.0839
 4.9589
[ CPUFloatType{6} ];

batch_sidelengths[13] =  2.2600
 4.4642
 4.6564
 5.5890
 5.1065
 4.9809
[ CPUFloatType{6} ];

irange of sample 8 = 3
dist_info.r_short_range = 2.06173
dist_info.r_lower = 2.25
dist_info.r_upper = 2.26
lower_energy = 0.000527995
upper_energy = 0.000534421
abinitio_energy = 0.000407018
mixed_energy = 0.000135302
```

```py
# before rescale: energy[12] = -0.42391228675842285
# after rescale:  energy[12] = 0.0005280036915634859
# before rescale: energy[13] = -0.42083460092544556
# after rescale:  energy[13] = 0.0005344150837873902

batch_sidelengths[12] = tensor([2.2500, 4.4444, 4.6358, 5.5642, 5.0839, 4.9589])
batch_sidelengths[13] = tensor([2.2600, 4.4642, 4.6564, 5.5890, 5.1065, 4.9809])

ir of sample 8 = InteractionRange.MIXED_SHORT
dist_info = ExtrapolationDistanceInfo(r_short_range=2.0617305525757135, r_lower=2.25, r_upper=2.26)
lower_energy = 0.0005280036712065339
upper_energy = 0.0005344150704331696
short_energy = 0.00040729661224501133
mixed_energy = 0.00013554721868472717
```

I don't know why this is happening, and for specifically this sample

One possibility is an issue with the permutation transformation?
  - in the first example, two of the side lengths are almost the same
  - so maybe the permutation swapping strategy is slightly different between the two models

But this doesn't explain the second case, where all the side lengths are unique
  - the output energies are off by just slightly enough, to be an issue after the rescaling and extrapolation


## Some ideas:
1. maybe set the tolerance level to `0.5 %`, if the energy is really small?
  - after all, those are the ones that matter the least
  - and I can't seem to find a reason for the difference!!!
2. make the permutation transformations in the two versions match
