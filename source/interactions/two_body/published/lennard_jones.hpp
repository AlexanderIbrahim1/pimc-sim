#pragma once

#include <concepts>

#include <interactions/two_body/two_body_pointwise.hpp>


namespace interact
{

template <std::floating_point FP>
constexpr auto lennard_jones_warnecke2010() -> interact::LennardJonesPotential<FP>
{
    /*
        Parameters for the Lennard-Jones potential are taken from paragraph 3 of page 354
        of `Eur. Phys. J. D 56, 353–358 (2010)`. Original units are in Kelvin and Angstroms,
        converted to wavenumbers and angstroms.
    */
    const auto well_depth = static_cast<FP> {23.77};
    const auto particle_size = static_cast<FP> {2.96};

    return interact::LennardJonesPotential {well_depth, particle_size};
}

}  // namespace interact
