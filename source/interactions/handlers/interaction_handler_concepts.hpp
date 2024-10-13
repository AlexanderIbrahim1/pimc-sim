#pragma once

#include <cstddef>
#include <concepts>

#include <mathtools/grid/square_adjacency_matrix.hpp>
#include <worldline/worldline.hpp>

namespace interact
{

template <typename Handler, typename FP, std::size_t NDIM>
concept InteractionHandler = requires(Handler t) {
    requires std::is_floating_point_v<FP>;
    {
        t(std::size_t {}, std::size_t {}, worldline::Worldlines<FP, NDIM> {std::size_t {}, std::size_t {}})
    } -> std::same_as<FP>;
};

template <typename Handler, typename FP, std::size_t NDIM>
concept NearestNeighbourInteractionHandler = requires(Handler t) {
    requires std::is_floating_point_v<FP>;
    {
        t(std::size_t {}, std::size_t {}, worldline::Worldlines<FP, NDIM> {std::size_t {}, std::size_t {}})
    } -> std::same_as<FP>;

    {
        t.adjacency_matrix()
    } -> std::same_as<mathtools::SquareAdjacencyMatrix&>;
};

}  // namespace interact
