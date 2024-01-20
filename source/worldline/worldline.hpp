#pragma once

#include <concepts>
#include <initializer_list>
#include <vector>

#include <coordinates/coordinates.hpp>

namespace
{

template <typename Container>
concept IterableContainer = requires(Container c) {
    std::begin(c);
    std::end(c);
    typename Container::value_type;
};

}  // namespace

namespace worldline
{

template <std::floating_point FP, std::size_t NDIM>
class Worldline
{
public:
    using Point = coord::Cartesian<FP, NDIM>;

    Worldline() = default;

    Worldline(std::initializer_list<Point> ilist)
        : points_ {ilist}
    {}

    template <IterableContainer Container>
    Worldline(const Container& container)
        : points_ {std::begin(container), std::end(container)}
    {
        static_assert(std::same_as<typename Container::value_type, Point>);
    }

    constexpr auto points() const -> const std::vector<Point>&
    {
        return points_;
    }

private:
    std::vector<Point> points_ {};
};

}  // namespace worldline
