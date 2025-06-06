#pragma once

#include <concepts>
#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>

#include <common/writer_utils.hpp>

namespace interact
{

template <std::floating_point FP>
void ctr_check_well_depth_positive(FP well_depth)
{
    if (well_depth <= FP {0.0}) {
        const auto precision = common::writers::DEFAULT_WRITER_FLOATING_POINT_PRECISION;

        auto err_msg = std::stringstream {};
        err_msg << "The Lennard-Jones well depth must be positive\nFound: ";
        err_msg << std::scientific << std::setprecision(precision) << well_depth << '\n';
        throw std::runtime_error(err_msg.str());
    }
}

template <std::floating_point FP>
void ctr_check_particle_size_positive(FP particle_size)
{
    if (particle_size <= FP {0.0}) {
        const auto precision = common::writers::DEFAULT_WRITER_FLOATING_POINT_PRECISION;

        auto err_msg = std::stringstream {};
        err_msg << "The Lennard-Jones particle size must be positive\nFound: ";
        err_msg << std::scientific << std::setprecision(precision) << particle_size << '\n';
        throw std::runtime_error(err_msg.str());
    }
}

template <std::floating_point FP>
class LennardJonesPotential
{
public:
    explicit LennardJonesPotential(FP well_depth, FP particle_size)
        : well_depth4_ {FP {4.0} * well_depth}
        , particle_size_ {particle_size}
    {
        ctr_check_well_depth_positive(well_depth);
        ctr_check_particle_size_positive(particle_size);
    }

    constexpr auto operator()(FP distance) const noexcept -> FP
    {
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
