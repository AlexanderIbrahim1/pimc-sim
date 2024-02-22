#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "interactions/two_body/two_body_pointwise_tabulated.hpp"

/*
I FOUND THIS FROM A STACKOVERFLOW COMMENT

inline std::string resolvePath(const std::string &relPath)
{
    namespace fs = std::tr2::sys;
    // or namespace fs = boost::filesystem;
    auto baseDir = fs::current_path();
    while (baseDir.has_parent_path())
    {
        auto combinePath = baseDir / relPath;
        if (fs::exists(combinePath))
        {
            return combinePath.string();
        }
        baseDir = baseDir.parent_path();
    }
    throw std::runtime_error("File not found!");
}

To use it, I go:

std::string foofullPath = resolvePath("test/data/foo.txt");

*/

TEST_CASE("basic FSH interaction check")
{
    const auto filepath = std::filesystem::path {"potentials"} / "fsh_potential_angstroms_wavenumbers.potext_sq";
    constexpr auto lr_check_status = interact::LongRangeCheckStatus::OFF;
    const auto fsh_potential = interact::create_fsh_pair_potential<double, lr_check_status>(filepath);

    // values are taken directly from the file
    SECTION("interpolate0")
    {
        const auto dist_sq = double {1.008100810081008092e+01};
        const auto expected_energy = double {-1.484086161650269275e+01};
        const auto actual_energy = fsh_potential(dist_sq);

        REQUIRE_THAT(expected_energy, Catch::Matchers::WithinRel(actual_energy));
    }
}
