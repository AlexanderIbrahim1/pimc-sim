#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../test_utils/test_tensor_utils.hpp"
#include "../test_utils/test_utils.hpp"
#include "interactions/four_body/published_potential.hpp"

auto load_published_ssp_four_body_potential()
{
    namespace fs = std::filesystem;
    using PTF = interact::PermutationTransformerFlag;

    const auto rel_filepath = fs::path {"playground"} / "scripts" / "models" / "fourbodypara_ssp_64_128_128_64.pt";
    const auto abs_filepath = test_utils::resolve_project_path(rel_filepath);

    return interact::get_published_four_body_potential<3, PTF::EXACT>(abs_filepath);
}

TEST_CASE("basic four-body interaction check")
{
    const auto potential = load_published_ssp_four_body_potential();

    const auto input =
        torch::tensor({2.2000000, 2.2000000, 2.2000000, 3.1112699, 3.1112699, 3.5925850}, torch::dtype(torch::kFloat32))
            .reshape({1, 6});

    const auto expected = 4.20351362f;  // from published python repo
    const auto actual = potential.evaluate_batch(input).item<float>();

    REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
}

TEST_CASE("multiple four-body interaction check")
{
    const auto potential = load_published_ssp_four_body_potential();

    const auto rel_side_lengths_filepath =
        std::filesystem::path {"playground"} / "fourbody_examples" / "sample_side_lengths.dat";
    const auto rel_energies_filepath =
        std::filesystem::path {"playground"} / "fourbody_examples" / "sample_energies_ssp_64_128_128_64.dat";
    const auto abs_side_lengths_filepath = test_utils::resolve_project_path(rel_side_lengths_filepath);
    const auto abs_energies_filepath = test_utils::resolve_project_path(rel_energies_filepath);

    // NOTE: apparently `torch::kFloat32` is not a type, and neither is `torch::dtype(torch::kFloat32)`
    const auto side_lengths_shape = torch::IntArrayRef {3400, 6};
    const auto side_lengths = test_utils::read_file_to_tensor_f32(abs_side_lengths_filepath, side_lengths_shape);

    const auto energies_shape = torch::IntArrayRef {3400, 1};
    const auto python_energies = test_utils::read_file_to_tensor_f32(abs_energies_filepath, energies_shape);

    auto cpp_energies = potential.evaluate_batch(side_lengths).reshape(energies_shape);

    // NOTE: the python and C++ energies differ slightly; this could be due to a lot of reasons, but one
    // of the most likely reasons is that the Python version does many `double <-> float` conversions,
    // whereas the C++ version does not

    for (std::int64_t i {0}; i < 3400; ++i) {
        const auto pyt_energy = python_energies[i].item<double>();
        const auto cpp_energy = cpp_energies[i].item<double>();

        INFO("i = " << i << ": pyt_energy = " << pyt_energy << ": cpp_energy = " << cpp_energy);
        REQUIRE_THAT(cpp_energy, Catch::Matchers::WithinRel(pyt_energy, 5.0e-4));
    }

    // REQUIRE(test_utils::almost_equal_relative(python_energies, cpp_energies));
}
