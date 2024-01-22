#pragma once

#include <concepts>

#include <common/common_utils.hpp>

namespace coord_utils
{

template <common_utils::IterableContainer Container>
constexpr void check_all_entries_are_positive(const Container& container)
{
    const auto is_nonpositive = [](auto x) { return x <= 0.0; };
    if (std::any_of(std::begin(container), std::end(container), is_nonpositive)) {
        throw std::runtime_error("All the box sides in a `BoxSides` instance must be positive.");
    }
}

}  // namespace coord_utils
