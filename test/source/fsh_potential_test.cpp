#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../test_utils/filesearch.hpp"
#include "interactions/two_body/two_body_pointwise_tabulated.hpp"

TEST_CASE("basic FSH interaction check")
{
    namespace fs = std::filesystem;

    const auto rel_filepath = fs::path {"potentials"} / "fsh_potential_angstroms_wavenumbers.potext_sq";
    const auto abs_filepath = test_utils::resolve_project_path(rel_filepath);
    constexpr auto lr_check_status = interact::LongRangeCheckStatus::OFF;
    const auto fsh_potential = interact::create_fsh_pair_potential<double, lr_check_status>(abs_filepath);

    // values are taken directly from the file
    struct TestPair
    {
        double input {};
        double expected {};
    };

    auto pairs = GENERATE(
        TestPair {1.008100810081008092e+01, -1.484086161650269275e+01},
        TestPair {1.200120012001200109e+01, -2.422224453532016497e+01},
        TestPair {2.992299229922992510e+01, -2.600928751906699699e+00},
        TestPair {4.752475247524752433e+01, -6.056475507234712063e-01}
    );

    const auto actual = fsh_potential(pairs.input);
    REQUIRE_THAT(pairs.expected, Catch::Matchers::WithinRel(actual));
}
