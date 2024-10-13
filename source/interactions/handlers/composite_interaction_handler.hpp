#pragma once

#include <concepts>
#include <cstddef>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include <interactions/handlers/interaction_handler_concepts.hpp>
#include <mathtools/grid/square_adjacency_matrix.hpp>
#include <worldline/worldline.hpp>

/*
TODO: create a class that takes a variadic number of other interaction handlers and combines
them into a single one
  - there might have to be multiple types (because the nearest-neighbour ones have additional operations, etc)
  - but this is a good start
*/

namespace interact
{

template <std::floating_point FP, std::size_t NDIM, typename... Handlers>
class CompositeFullInteractionHandler
{
public:
    CompositeFullInteractionHandler(Handlers... handlers)
        : handlers_ {std::move(handlers)...}
    {
        static_assert(
            sizeof...(handlers) >= 1, "There must be at least one handler in the CompositeInteractionHandler"
        );
        // static_assert(InteractionHandler<Handlers>..., "All inputs must be InteractionHandlers");
        static_assert(std::conjunction<std::bool_constant<InteractionHandler<Handlers, FP, NDIM>>...>::value, "");
    }

    constexpr auto operator()(
        std::size_t i_timeslice,
        std::size_t i_particle,
        const worldline::Worldlines<FP, NDIM>& worldlines
    ) const noexcept -> FP
    {
        auto pot_energy = FP {};
        const auto handler_looper = [&](auto&&... handler)
        { ((pot_energy += handler(i_timeslice, i_particle, worldlines)), ...); };

        std::apply(handler_looper, handlers_);

        return pot_energy;
    }

private:
    std::tuple<Handlers...> handlers_;
};

template <std::floating_point FP, std::size_t NDIM, typename... Handlers>
class CompositeNearestNeighbourInteractionHandler
{
public:
    CompositeNearestNeighbourInteractionHandler(Handlers... handlers)
        : handlers_ {std::move(handlers)...}
    {
        static_assert(
            sizeof...(handlers) >= 1, "There must be at least one handler in the CompositeInteractionHandler"
        );
        static_assert(
            std::conjunction<std::bool_constant<NearestNeighbourInteractionHandler<Handlers, FP, NDIM>>...>::value, ""
        );
    }

    constexpr auto operator()(
        std::size_t i_timeslice,
        std::size_t i_particle,
        const worldline::Worldlines<FP, NDIM>& worldlines
    ) noexcept -> FP
    {
        auto pot_energy = FP {};
        const auto handler_looper = [&](auto&&... handler)
        { ((pot_energy += handler(i_timeslice, i_particle, worldlines)), ...); };

        std::apply(handler_looper, handlers_);

        return pot_energy;
    }

    template <std::size_t Index>
    constexpr auto adjacency_matrix() noexcept -> mathtools::SquareAdjacencyMatrix&
    {
        auto& handler = std::get<Index>(handlers_);
        return handler.adjacency_matrix();
    }

    template <std::size_t Index>
    constexpr auto get() noexcept -> NearestNeighbourInteractionHandler<FP, NDIM> auto&
    {
        return std::get<Index>(handlers_);
    }

private:
    std::tuple<Handlers...> handlers_;
};

}  // namespace interact
