#include <iostream>
#include <string>
#include <string_view>

#include <tomlplusplus/toml.hpp>

#include <coordinates/coordinates.hpp>

auto main() -> int {
    const auto some_toml = std::string_view {R"(
        [library]
        name = "toml++"
        authors = ["Mark Gillard <mark.gillard@outlook.com.au>"]
        cpp = 17
    )"};

    try {
        // parse directly from a string view:
        {
            toml::table tbl = toml::parse(some_toml);
            std::cout << tbl << "\n";
        }

        // parse from a string stream:
        {
            std::stringstream ss {std::string {some_toml}};
            toml::table tbl = toml::parse(ss);
            std::cout << tbl << "\n";
        }
    }
    catch (const toml::parse_error& err) {
        std::cerr << "Parsing failed:\n" << err << "\n";
        return 1;
    }

    const auto point = coord::Cartesian<double, 2> {1.0, 2.0};
    std::cout << point.as_string() << '\n';

    return 0;
}
