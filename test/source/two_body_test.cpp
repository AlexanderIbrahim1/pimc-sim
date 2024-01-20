#include <cmath>
#include <concepts>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "interactions/two_body/two_body_pointwise.hpp"

constexpr auto EPSILON_ENERGY = double {1.0e-8};

TEST_CASE("lennard-jones potential : concept properties") {
    static_assert(interact::PairDistancePotential<interact::LennardJonesPotential<float>>,
                  "ERROR: the LennardJonesPotential was found not to satisfy the "
                  "PairDistancePotential concept!");
}

TEST_CASE("lennard-jones potential : physical properties", "[LennardJonesPotential]") {
    const auto well_depth = 1.0;
    const auto particle_size = 1.0;
    const auto pot = interact::LennardJonesPotential<double>(well_depth, particle_size);

    const auto distance_of_minimum = std::pow(2.0, 1.0 / 6.0) * particle_size;

    SECTION("check minimum value") {
        const auto energy_at_minimum = pot(distance_of_minimum);

        REQUIRE_THAT(energy_at_minimum, Catch::Matchers::WithinRel(-well_depth));
    }

    SECTION("check minimum surroundings") {
        const auto left_of_minimum = distance_of_minimum * 0.98;
        const auto right_of_minimum = distance_of_minimum * 1.02;

        REQUIRE(pot(distance_of_minimum) < pot(left_of_minimum));
        REQUIRE(pot(distance_of_minimum) < pot(right_of_minimum));
    }

    SECTION("check energy at particle size") {
        const auto energy_at_particle_size = pot(particle_size);

        REQUIRE_THAT(energy_at_particle_size, Catch::Matchers::WithinAbs(0.0, EPSILON_ENERGY));
    }

    SECTION("check energy at particle size surroundings") {
        const auto left_of_particle_size = particle_size * 0.98;
        const auto right_of_particle_size = particle_size * 1.02;

        REQUIRE(pot(left_of_particle_size) > 0.0);
        REQUIRE(pot(right_of_particle_size) < 0.0);
    }
}
