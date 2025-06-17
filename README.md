# pimc-sim

This is the pimc-sim project.
It is a template-heavy, header-only codebase,
whose goal is to perform PIMC simulations of solid parahydrogen using non-additive many-body interactions potentials.
These simulations are performed for a wide range of densities and simulation settings.
The templated nature of the codebase allows simulations to performed at any temperature,
for any number of dimensions,
and for both single-precision and double-precision floating-point numbers.

This codebase was used to perform the path-integral Monte Carlo (PIMC) simulations for the paper "[Path-integral Monte Carlo simulations of solid parahydrogen using two-body, three-body, and four-body
*ab initio* interaction potential energy surfaces](https://pubs.aip.org/aip/jcp/article-abstract/162/16/164503/3344880/Path-integral-Monte-Carlo-simulations-of-solid?redirectedFrom=fulltext)."

Also accessible via [arxiv](https://arxiv.org/abs/2506.05557).

## Example of use
An example of the code being used is in `pimc-sim/source/main.cpp`.

The input to the executable is TOML file.
This file contains parameters that determine the physical properties of the lattice,
such as the temperature, the number of unit cells, and the density.
It also contains the Monte Carlo simulation parameters,
such as the number of passes per block,
the number of blocks,
the Monte Carlo step sizes,
and so on.
An example of the parameters shown is given in `pimc-sim/source/argparser.hpp`.

## TODO
- continue the README
- clean up the `.cpp` files
