#pragma once

#include <concepts>
#include <tuple>

#include <interactions/handlers/interaction_handler_concepts.hpp>

/*
TODO: create a class that takes a variadic number of other interaction handlers and combines
them into a single one
  - there might have to be multiple types (because the nearest-neighbour ones have additional operations, etc)
  - but this is a good start
*/

namespace interact
{

template <typename Handler, std::floating_point FP, std::size_t NDIM>
class CompositeInteractionHandler
{
public:

private:

};
    
}  // namespace interact
