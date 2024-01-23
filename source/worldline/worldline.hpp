#pragma once

#include <concepts>
#include <initializer_list>
#include <vector>

#include <common/common_utils.hpp>
#include <coordinates/coordinates.hpp>

namespace worldline
{

template <std::floating_point FP, std::size_t NDIM>
class Worldline
{
public:
    using Point = coord::Cartesian<FP, NDIM>;

    constexpr Worldline() = default;

    constexpr Worldline(std::initializer_list<Point> ilist)
        : points_ {ilist}
    {}

    template <common_utils::IterableContainer Container>
    constexpr Worldline(const Container& container)
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
