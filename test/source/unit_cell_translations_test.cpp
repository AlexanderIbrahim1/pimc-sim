#include <array>
#include <cstddef>

#include <catch2/catch_test_macros.hpp>

#include "geometries/unit_cell_translations.hpp"

static constexpr auto operator""_uz(unsigned long long n) -> std::size_t
{
    return std::size_t {n};
}

TEST_CASE("unit cell translations : basic", "[UnitCellTranslations]")
{
    const auto unit_cell_trans = geom::UnitCellTranslations<3> {2_uz, 3_uz, 4_uz};

    SECTION("correct translations")
    {
        REQUIRE(unit_cell_trans.translations() == std::array<std::size_t, 3> {2, 3, 4});
    }

    SECTION("correct total number of boxes")
    {
        REQUIRE(geom::n_total_boxes(unit_cell_trans) == std::size_t {2 * 3 * 4});
    }
}

TEST_CASE("unit cell translations : throw with zero values", "[UnitCellTranslations]")
{
    REQUIRE_THROWS_AS(geom::UnitCellTranslations<3>(0_uz, 1_uz, 1_uz), std::runtime_error);
}

TEST_CASE("unit cell increments", "[UnitCellIncrementer]")
{
    using arr = std::array<std::size_t, 3>;
    const auto unit_cell_trans = geom::UnitCellTranslations<3> {2_uz, 3_uz, 4_uz};

    auto incrementer = geom::UnitCellIncrementer<3> {unit_cell_trans};

    // clang-format off
    const auto expected_indices = std::vector<arr> {
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}, {0, 2, 0}, {1, 2, 0},
        {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}, {0, 2, 1}, {1, 2, 1},
        {0, 0, 2}, {1, 0, 2}, {0, 1, 2}, {1, 1, 2}, {0, 2, 2}, {1, 2, 2},
        {0, 0, 3}, {1, 0, 3}, {0, 1, 3}, {1, 1, 3}, {0, 2, 3}, {1, 2, 3},
        {0, 0, 0}
    };
    // clang-format on

    for (const auto& index : expected_indices) {
        REQUIRE(incrementer.indices() == index);
        incrementer.increment();
    }
}
