#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "interactions/four_body/published_potential.hpp"
#include "../test_utils/test_utils.hpp"

TEST_CASE("basic four-body interaction check")
{
    namespace fs = std::filesystem;

    using PTF = interact::PermutationTransformerFlag;

    const auto rel_filepath = fs::path {"playground"} / "scripts" / "models" / "fourbodypara_ssp_64_128_128_64.pth";
    const auto abs_filepath = test_utils::resolve_project_path(rel_filepath);
    const auto potential = interact::get_published_four_body_potential<3, PTF::EXACT>(abs_filepath);

    const auto input = torch::tensor(
        {2.2000000, 2.2000000, 2.2000000, 3.1112699, 3.1112699, 3.5925850},
        torch::dtype(torch::kFloat32)).reshape({1, 6}
    );

    const auto expected = 4.20351362f;
    const auto actual = potential.evaluate_batch(input).item<float>();

    REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));

    // // values are taken directly from the file
    // struct TestPair
    // {
    //     double input {};
    //     double expected {};
    // };

    // auto pairs = GENERATE(
    //     TestPair {1.008100810081008092e+01, -1.484086161650269275e+01},
    //     TestPair {1.200120012001200109e+01, -2.422224453532016497e+01},
    //     TestPair {2.992299229922992510e+01, -2.600928751906699699e+00},
    //     TestPair {4.752475247524752433e+01, -6.056475507234712063e-01}
    // );

    // const auto actual = fsh_potential(pairs.input);
    // REQUIRE_THAT(pairs.expected, Catch::Matchers::WithinRel(actual));
}
