#pragma once

#include <concepts>

namespace interact
{

template <typename T>
concept InteractionHandler = requires(T t) {
    {
        t(0, {})
    } -> std::floating_point;
};

}  // namespace interact
