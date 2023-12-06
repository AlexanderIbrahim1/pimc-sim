#pragma once

#include <concepts>
#include <vector>

namespace worldline
{

template <std::floating_point FP>
class Worldline
{
public:

private:
    std::vector<FP> points;
};

}  // namespace worldline