#pragma once

#include <concepts>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <mathtools/interpolate/linear_interp.hpp>

namespace interact
{

enum class LongRangeCheckStatus
{
    ON,
    OFF
};

template <std::floating_point FP>
constexpr auto ctr_calculate_c6_multipole_coeff(const std::vector<FP>& energies, FP r2_min, FP r2_max) -> FP
{
    // NOTES:
    // - not too many useful names for the temporary variables created in this function
    // - the interpolator will have thrown an exception if size < 2, so vector accesses should be okay
    const auto size = energies.size();
    const auto r2_step_size = (r2_max - r2_min) / static_cast<FP>(size - 1);

    const auto energy_step = energies[size - 1] - energies[size - 2];

    const auto r2_last = r2_max;
    const auto r2_sec_last = r2_max - r2_step_size;

    const auto r2_term0 = r2_sec_last * r2_sec_last * r2_sec_last;
    const auto r2_term1 = r2_last * r2_last * r2_last;

    return energy_step / (FP {1.0} / r2_term0 - FP {1.0} / r2_term1);
}

template <std::floating_point FP, LongRangeCheckStatus Status>
class FSHTwoBodyPotentialBase
{
public:
    explicit FSHTwoBodyPotentialBase(std::vector<FP> energies, FP r2_min, FP r2_max)
        : c6_multipole_coeff_ {ctr_calculate_c6_multipole_coeff(energies, r2_min, r2_max)}
        , interpolator_ {std::move(energies), r2_min, r2_max}
        , r2_max_ {r2_max}
    {}

    constexpr auto operator()(FP dist_squared) const noexcept -> FP
    {
        if constexpr (Status == LongRangeCheckStatus::OFF) {
            return interpolator_(dist_squared);
        }
        else {
            if (dist_squared >= r2_max_) {
                const auto dist_pow6 = dist_squared * dist_squared * dist_squared;
                return c6_multipole_coeff_ / (dist_pow6);
            }
            else {
                return interpolator_(dist_squared);
            }
        }
    }

private:
    FP c6_multipole_coeff_;
    mathtools::RegularLinearInterpolator<FP> interpolator_;
    FP r2_max_;
};

template <std::floating_point FP>
class FSHTwoBodyPotential : public FSHTwoBodyPotentialBase<FP, LongRangeCheckStatus::OFF>
{
    using FSHTwoBodyPotentialBase<FP, LongRangeCheckStatus::OFF>::FSHTwoBodyPotentialBase;
};

static auto number_of_lines(const std::filesystem::path& filepath) -> std::size_t
{
    auto instream = std::ifstream {filepath, std::ios::in};

    if (!instream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Unable to open file '" << filepath.string() << "'.\n";
        throw std::runtime_error(err_msg.str());
    }

    auto n_lines = std::size_t {0};
    auto dummy = std::string {};
    while (std::getline(instream, dummy)) {
        ++n_lines;
    }

    return n_lines;
}

template <std::floating_point FP>
auto read_one_distance_squared_and_energy(std::ifstream& instream) -> std::tuple<FP, FP>
{
    auto line = std::string {};
    std::getline(instream, line);

    auto valuestream = std::istringstream {line};

    auto dist_squared = FP {};
    auto energy = FP {};

    valuestream >> dist_squared;
    valuestream >> energy;

    return {dist_squared, energy};
}

template <std::floating_point FP>
auto two_body_schmidt2015(const std::filesystem::path& fsh_filepath) -> FSHTwoBodyPotential<FP>
{
    /*
        The two-body pair potential for two parahydrogen molecules. Taken from the paper
        `J. Phys. Chem. A 199, 12551 (2015).

        The potential takes inputs in the form of the pair distance squared, in units of
        Angstroms, and returns outputs in the form of the interaction energy, in units of
        wavenumbers.
    */
    auto instream = std::ifstream {fsh_filepath, std::ios::in};

    if (!instream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Error: Unable to open file " << fsh_filepath.string() << ".\n";
        throw std::runtime_error(err_msg.str());
    }

    const auto size = number_of_lines(fsh_filepath);

    auto energies = std::vector<FP> {};
    energies.reserve(size);

    // interested in the first value of the pair distance squared
    const auto [r2_min, energy0] = read_one_distance_squared_and_energy<FP>(instream);
    energies.push_back(energy0);

    // now read in all energies except for the last
    auto r2_discard = FP {};
    auto energy = FP {};
    auto line = std::string {};
    for (std::size_t i {1}; i < size - 1; ++i) {
        std::getline(instream, line);
        auto valuestream = std::istringstream {line};

        valuestream >> r2_discard;
        valuestream >> energy;
        energies.push_back(energy);
    }

    // interested in the last value of the pair distance squared
    const auto [r2_max, energy_last] = read_one_distance_squared_and_energy<FP>(instream);
    energies.push_back(energy_last);

    return FSHTwoBodyPotential<FP> {std::move(energies), r2_min, r2_max};
}

}  // namespace interact
