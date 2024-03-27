#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mathtools/grid/grid3d.hpp"
#include "mathtools/interpolate/trilinear_interp.hpp"
#include "mathtools/mathtools_utils.hpp"

constexpr auto trilinear_function(double x, double y, double z) noexcept -> double
{
    // if we use a trilinear function, then the trilinear interpolator should give us
    // back the exact values of the trilinear function during interpolation.
    return 1.0 + 2.0 * x + 3.0 * y + 4.0 * z;
}

auto create_234_grid() noexcept -> mathtools::Grid3D<double>
{
    const auto shape = mathtools::Shape3D {3, 4, 5};

    auto grid = mathtools::Grid3D<double> {shape};
    for (std::size_t i0 {0}; i0 < shape.size0; ++i0) {
        for (std::size_t i1 {0}; i1 < shape.size1; ++i1) {
            for (std::size_t i2 {0}; i2 < shape.size2; ++i2) {
                const auto x = static_cast<double>(i0);
                const auto y = static_cast<double>(i1);
                const auto z = static_cast<double>(i2);
                const auto value = trilinear_function(x, y, z);
                grid.set(i0, i1, i2, value);
            }
        }
    }

    return grid;
}

TEST_CASE("test basic trilinear interpolation", "[TrilinearInterpolator]")
{
    const auto grid = create_234_grid();
    const auto interpolator = mathtools::TrilinearInterpolator<double> {
        grid, {0.0, 2.0},
         {0.0, 3.0},
         {0.0, 4.0}
    };

    const auto dx = double {0.2};
    const auto dy = double {0.3};
    const auto dz = double {0.4};

    for (std::size_t ix {0}; ix < 11; ++ix) {
        for (std::size_t iy {0}; iy < 11; ++iy) {
            for (std::size_t iz {0}; iz < 11; ++iz) {
                const auto x = static_cast<double>(ix) * dx;
                const auto y = static_cast<double>(iy) * dy;
                const auto z = static_cast<double>(iz) * dz;

                const auto expected = trilinear_function(x, y, z);
                const auto actual = interpolator(x, y, z);

                REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
            }
        }
    }
}
