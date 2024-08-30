#include <array>
#include <random>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "rng/distributions.hpp"
#include "rng/generator.hpp"
#include "rng/prng_state.hpp"


auto generate_n_integers(rng::RandomNumberGeneratorWrapper<std::mt19937>& prngw, std::size_t size, int max) -> std::vector<int>
{
    std::vector<int> output;
    output.reserve(size);

    auto uint_distrib = rng::UniformIntegerDistribution<int> {};
    for (std::size_t i {0}; i < size; ++i) {
        const auto value = uint_distrib.uniform_0n(max, prngw);
        output.push_back(value);
    }

    return output;
}

TEST_CASE("save and load std::mt19937 state", "[rng]") {
    auto prngw0 = rng::RandomNumberGeneratorWrapper<std::mt19937>::from_random_uint64();
    auto prngw1 = rng::RandomNumberGeneratorWrapper<std::mt19937>::from_uint64(0);
    const auto size = std::size_t {10};
    const auto max = int {100};

    auto state_stream = std::stringstream {};

    state_stream << prngw0.prng();
    const auto output0 = generate_n_integers(prngw0, size, max);


    state_stream >> prngw1.prng();
    const auto output1 = generate_n_integers(prngw1, size, max);

    REQUIRE(output0 == output1);
}
