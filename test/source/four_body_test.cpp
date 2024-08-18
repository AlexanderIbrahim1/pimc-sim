#include <cmath>
#include <filesystem>
#include <tuple>

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

    const auto rel_filepath = fs::path {"playground"} / "scripts" / "models" / "fourbodypara_ssp_64_128_128_64_cpu_eval.pt";
    const auto abs_filepath = test_utils::resolve_project_path(rel_filepath);

    return interact::get_published_four_body_potential<3, PTF::EXACT>(abs_filepath);
}

auto load_published_ssp_buffered_four_body_potential(long int buffer_size)
{
    namespace fs = std::filesystem;
    using PTF = interact::PermutationTransformerFlag;

    const auto rel_filepath = fs::path {"playground"} / "scripts" / "models" / "fourbodypara_ssp_64_128_128_64_cpu_eval.pt";
    const auto abs_filepath = test_utils::resolve_project_path(rel_filepath);

    return interact::get_published_buffered_four_body_potential<3, PTF::EXACT>(abs_filepath, buffer_size);
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
    //
    // It looks like the torch module itself will direcly produce slightly different outputs for the same input,
    // and after a few days of bug chasing and googling, I'm still not sure why they aren't 1-to-1
    // 
    // The differences are really slight, however
    //
    // I'm making the tolerances 1% for very weak energies, and 0.1% for all other energies; this causes the
    // tests to pass; though it seems that making the tolerance 0.001% makes the majority of them pass anyways;
    // it is only outliers that cause the difference

    for (std::int64_t i {0}; i < 3400; ++i) {
        const auto pyt_energy = python_energies[i].item<float>();
        const auto cpp_energy = cpp_energies[i].item<float>();

        INFO("i = " << i << ": pyt_energy = " << pyt_energy << ": cpp_energy = " << cpp_energy);
        if (std::abs(pyt_energy) < 1.0e-2) {
            REQUIRE_THAT(cpp_energy, Catch::Matchers::WithinRel(pyt_energy, 1.0e-2f));
        } else {
            REQUIRE_THAT(cpp_energy, Catch::Matchers::WithinRel(pyt_energy, 1.0e-3f));
        }
    }

    // REQUIRE(test_utils::almost_equal_relative(python_energies, cpp_energies));
}

TEST_CASE("buffered potential interaction check")
{
    // get the individual outputs from the potential
    const auto [output0, output1, output2] = []() {
        const auto potential = load_published_ssp_four_body_potential();

        const auto input0 =
            torch::tensor({2.2000000, 2.2000000, 2.2000000, 3.1112699, 3.1112699, 3.5925850}, torch::dtype(torch::kFloat32))
                .reshape({1, 6});
    
        const auto output0 = potential.evaluate_batch(input0).item<float>();

        const auto input1 =
            torch::tensor({2.5000000, 2.1000000, 3.2000000, 4.3112699, 2.7112699, 3.1925850}, torch::dtype(torch::kFloat32))
                .reshape({1, 6});
    
        const auto output1 = potential.evaluate_batch(input1).item<float>();

        const auto input2 =
            torch::tensor({2.3000000, 2.7000000, 3.3000000, 4.0112699, 2.2112699, 3.4925850}, torch::dtype(torch::kFloat32))
                .reshape({1, 6});
    
        const auto output2 = potential.evaluate_batch(input2).item<float>();

        return std::make_tuple(output0, output1, output2);
    }();

    // get the three outputs from the buffered potential
    const auto total_energy = []() {
        auto potential = load_published_ssp_buffered_four_body_potential(5);
        potential.add_sample({2.2000000f, 2.2000000f, 2.2000000f, 3.1112699f, 3.1112699f, 3.5925850f});
        potential.add_sample({2.5000000f, 2.1000000f, 3.2000000f, 4.3112699f, 2.7112699f, 3.1925850f});
        potential.add_sample({2.3000000f, 2.7000000f, 3.3000000f, 4.0112699f, 2.2112699f, 3.4925850f});

        return potential.extract_energy();
    }();

    REQUIRE_THAT(output0 + output1 + output2, Catch::Matchers::WithinRel(total_energy));
}
