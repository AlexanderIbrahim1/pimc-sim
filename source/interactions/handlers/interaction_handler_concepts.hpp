#pragma once

#include <concepts>

#include <mathtools/grid/square_adjacency_matrix.hpp>

namespace interact
{

template <typename T>
concept InteractionHandler = requires(T t) {
    {
        t(0, {})
    } -> std::floating_point;
};

template <typename T>
concept NearestNeighbourInteractionHandler = requires(T t) {
    {
        t(0, {})
    } -> std::floating_point;

    {
        t.adjacency_matrix()
    } -> std::same_as<mathtools::SquareAdjacencyMatrix&>;
};

}  // namespace interact
