#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mathtools/grid/grid3d.hpp"
#include "mathtools/interpolate/trilinear_interp.hpp"
#include "mathtools/mathtools_utils.hpp"

constexpr auto trilinear_function(double x, double y, double z) noexcept -> double {
    // if we use a trilinear function, then the trilinear interpolator should give us
    // back the exact values of the trilinear function during interpolation.
    return 1.0 + 2.0 * x + 3.0 * y + 4.0 * z;
}


TEST_CASE("test basic trilinear interpolation", "[TrilinearInterpolator]") {
    const auto shape = mathtools::Shape3D {3, 4, 5};

    const auto grid = [&shape]() {
        auto grid_ = mathtools::Grid3D<double>{shape};
        for (std::size_t i0 {0}; i0 < shape.size0; ++i0) {
            for (std::size_t i1 {0}; i1 < shape.size1; ++i1) {
                for (std::size_t i2 {0}; i2 < shape.size2; ++i2) {
                    const auto x = static_cast<double>(i0);
                    const auto y = static_cast<double>(i1);
                    const auto z = static_cast<double>(i2);
                    const auto value = trilinear_function(x, y, z);
                    grid_.set(i0, i1, i2, value);
                }
            }
        }

        return grid_;
    }();

    const auto interpolator = mathtools::TrilinearInterpolator<double>{grid, {0.0, 3.0}, {0.0, 4.0}, {0.0, 5.0}};

    const auto x = 1.0;
    const auto y = 1.0;
    const auto z = 1.0;
    const auto expected = trilinear_function(x, y, z);
    const auto actual = interpolator(x, y, z);

    REQUIRE_THAT(expected, Catch::Matchers::WithinRel(actual));
}
