#pragma once

#include <cmath>
#include <concepts>
#include <stdexcept>

namespace geom
{

enum class LatticeType
{
    HCP
};

template <std::floating_point FP>
auto density_to_lattice_constant(FP density, LatticeType lattype) -> FP
{
    using LT = LatticeType;

    switch (lattype) {
        case LT::HCP : {
            return std::cbrt(std::sqrt(FP {2.0}) / density);
        }
        default : {
            throw std::runtime_error(
                "Encountered a lattice type for which there was no mapping of density to lattice constant.\n"
            );
        }
    };
}

}  // namespace geom
