#pragma once

#include <concepts>
#include <format>
#include <stdexcept>
#include <string>

namespace interact
{

template <typename Potential>
concept PairDistancePotential = requires(Potential pot) {
    { pot(0.0) } -> std::floating_point;
};

template <std::floating_point FP>
void ctr_check_well_depth_positive(FP well_depth) {
    if (well_depth <= FP {0.0}) {
        const std::string msg = std::format(
            "The Lennard-Jones well depth must be positive\nFound: {: .6e}", well_depth);
        throw std::runtime_error(msg);
    }
}

template <std::floating_point FP>
void ctr_check_particle_size_positive(FP particle_size) {
    if (particle_size <= FP {0.0}) {
        const std::string msg = std::format(
            "The Lennard-Jones particle size must be positive\nFound: {: .6e}", particle_size);
        throw std::runtime_error(msg);
    }
}

template <std::floating_point FP>
class LennardJonesPotential
{
public:
    explicit LennardJonesPotential(FP well_depth, FP particle_size)
        : well_depth4_ {FP {4.0} * well_depth}
        , particle_size_ {particle_size} {
        ctr_check_well_depth_positive(well_depth);
        ctr_check_particle_size_positive(particle_size);
    }

    constexpr auto operator()(FP distance) const noexcept -> FP {
        const auto s = particle_size_ / distance;
        const auto s3 = s * s * s;
        const auto s6 = s3 * s3;
        const auto s12 = s6 * s6;

        return well_depth4_ * (s12 - s6);
    }

private:
    FP well_depth4_;
    FP particle_size_;
};

}  // namespace interact
