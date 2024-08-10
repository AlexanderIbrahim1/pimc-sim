#include <cmath>
#include <concepts>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "coordinates/cartesian.hpp"
#include "interactions/four_body/constants.hpp"
#include "interactions/four_body/dispersion_potential.hpp"

// def get_tetrahedron_points(sidelen: float) -> list[Cartesian3D]:
//     p0 = sidelen * Cartesian3D(-0.5, 0.0, 0.0)
//     p1 = sidelen * Cartesian3D(0.5, 0.0, 0.0)
//     p2 = sidelen * Cartesian3D(0.0, math.sqrt(3.0 / 4.0), 0.0)
//     p3 = sidelen * Cartesian3D(0.0, math.sqrt(1.0 / 12.0), math.sqrt(2.0 / 3.0))
//
//     return [p0, p1, p2, p3]

template <std::floating_point FP>
constexpr auto get_tetrahedron_points(FP side_length)
{
    const auto p0 = side_length * coord::Cartesian<FP, 3> {-0.5, 0.0, 0.0};
    const auto p1 = side_length * coord::Cartesian<FP, 3> {0.5, 0.0, 0.0};
    const auto p2 = side_length * coord::Cartesian<FP, 3> {0.0, std::sqrt(3.0 / 4.0), 0.0};
    const auto p3 = side_length * coord::Cartesian<FP, 3> {0.0, std::sqrt(1.0 / 12.0), std::sqrt(2.0 / 3.0)};

    return std::tuple {p0, p1, p2, p3};
}

TEST_CASE("dispersion potential equality", "FourBodyDispersionPotential")
{
    const auto bade_coeff = interact::constants4b::BADE_COEFF_MIDZUNO_KIHARA<float>;
    const auto original_potential = interact::disp::FourBodyDispersionPotential<float, 3>(bade_coeff);
    const auto rescaled_potential = interact::disp::RescalingFourBodyDispersionPotential<float, 3>(bade_coeff);

    SECTION("shorter distances")
    {
        auto distance = GENERATE(2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 12.0f, 14.0f);
        const auto [p0, p1, p2, p3] = get_tetrahedron_points<float>(distance);
        const auto original_energy = original_potential(p0, p1, p2, p3);
        const auto rescaled_energy = rescaled_potential(p0, p1, p2, p3);

        INFO("original = " << original_energy);
        INFO("rescaled = " << rescaled_energy);
        REQUIRE_THAT(original_energy, Catch::Matchers::WithinRel(rescaled_energy));
    }
}
