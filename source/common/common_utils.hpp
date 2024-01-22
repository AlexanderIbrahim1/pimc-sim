#pragma once

#include <concepts>
#include <iterator>

namespace common_utils
{

template <typename Container>
concept IterableContainer = requires(Container c) {
    std::begin(c);
    std::end(c);
    typename Container::value_type;
};

}  // namespace common_utils
