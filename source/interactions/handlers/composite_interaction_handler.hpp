#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include <interactions/handlers/interaction_handler_concepts.hpp>
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
    using Worldline = worldline::Worldline<FP, NDIM>;

    CompositeFullInteractionHandler(Handlers... handlers)
        : _handlers {std::move(handlers)...}
    {
        static_assert(
            sizeof...(handlers) >= 1, "There must be at least one handler in the CompositeInteractionHandler"
        );
        // static_assert(InteractionHandler<Handlers>..., "All inputs must be InteractionHandlers");
        static_assert(std::conjunction<std::bool_constant<InteractionHandler<Handlers>>...>::value, "");
    }

    constexpr auto operator()(std::size_t i_particle, const Worldline& worldline) const noexcept -> FP
    {
        auto pot_energy = FP {};
        const auto handler_looper = [&](auto&&... handler) { ((pot_energy += handler(i_particle, worldline)), ...); };

        std::apply(handler_looper, _handlers);

        return pot_energy;
    }

private:
    std::tuple<Handlers...> _handlers;
};

}  // namespace interact
