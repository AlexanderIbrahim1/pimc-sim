#pragma once

// Many of the distribution functions in the <random> header require the parameters of the
// distribution to be known at compile-time. For example, 'std::normal_distribution' requires
// the mean and standard distribution upon instantiation.
//
// This header contains functions that allow the generation of random numbers following a
// distribution, where the parameters of the distribution are known at runtime.

#include <cassert>
#include <concepts>
#include <random>

#include <rng/generator.hpp>

namespace rng
{

// normal distribution
template <std::floating_point FP>
class NormalDistribution
{
public:
    constexpr NormalDistribution()
        : distrib_ {FP{0.0}, FP{1.0}}
    {}

    // number generated from a normal distribution with mean 0 and standard deviation 1
    constexpr auto normal_01(PRNGWrapper auto& prngw) noexcept -> FP {
        return distrib_(prngw.prng());
    }

    // number generated from a normal distirbution with a mean and standard deviation
    // decided at runtime
    constexpr auto normal(FP mean, FP stddev, PRNGWrapper auto& prngw) noexcept -> FP {
        assert(stddev > 0.0);
        const auto value = distrib_(prngw.prng());
        return stddev * value + mean;
    }

private:
    std::normal_distribution<FP> distrib_;
};


// uniform real distribution between two values
template <std::floating_point FP>
class UniformFloatingPointDistribution
{
public:
    constexpr UniformFloatingPointDistribution()
        : distrib_ {FP{0.0}, FP{1.0}}
    {}

    // number generated from a uniform distribution with a lower and upper bound decided at runtime
    // NOTE: this function works whether a > b or b > a
    //       that's why they're not named 'lower' and 'upper'
    constexpr auto uniform_ab(FP a, FP b, PRNGWrapper auto& prngw) noexcept -> FP {
        const auto value = distrib_(prngw.prng());
        return value * (b - a) + a;
    }

    // number generated from a uniform distribution between 0.0 and 1.0
    constexpr auto uniform_01(PRNGWrapper auto& prngw) noexcept -> FP {
        return distrib_(prngw.prng());
    }

private:
    std::uniform_real_distribution<FP> distrib_;
};


// uniform integer distribution
// We need to know what the lower and upper limits of the integers are at runtime, and thus we
// cannot use 'std::uniform_int_distribution'. Instead, we generate floating point numbers in
// the range [0, 1), rescale them, and case them to integers
template <std::integral Integer>
class UniformIntegerDistribution
{
private:
    std::uniform_real_distribution<double> m_unif01_distrib;

public:
    constexpr UniformIntegerDistribution()
        : m_unif01_distrib {0.0, 1.0} {}

    // generate a uniformly distributed integer in the range [a, b)
    // NOTE: if 'b < a', then the integer will end up in the range (b, a) instead; because this
    //       probably isn't something the user expects, we restrict the generator to a < b
    constexpr auto uniform_ab(Integer a, Integer b, PRNGWrapper auto& prngw) noexcept -> Integer {
        assert(a < b);
        const auto fp_a = static_cast<double>(a);
        const auto fp_b = static_cast<double>(b);
        const auto val01 = m_unif01_distrib(prngw.prng());
        const auto value = val01 * (fp_b - fp_a) + fp_a;

        return static_cast<Integer>(value);
    }

    // generate a uniformly distributed integer in the range [a, b]
    constexpr auto uniform_ab_inclusive(Integer a, Integer b, PRNGWrapper auto& prngw) noexcept -> Integer {
        return uniform_ab(a, b + 1, prngw);
    }

    // generate a uniformly distributed integer in the range [0, n)
    constexpr auto uniform_0n(Integer n, PRNGWrapper auto& prngw) noexcept -> Integer {
        const auto fp_n = static_cast<double>(n);
        const auto val01 = m_unif01_distrib(prngw.prng());
        const auto value = val01 * fp_n;

        return static_cast<Integer>(value);
    }

    // generate a uniformly distributed integer in the range [0, n]
    constexpr auto uniform_0n_inclusive(Integer n, PRNGWrapper auto& prngw) noexcept -> Integer {
        return uniform_0n(n + 1, prngw);
    }
};

}  // namespace rng
