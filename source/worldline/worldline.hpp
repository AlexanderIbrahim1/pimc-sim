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

    constexpr auto points() const noexcept -> const std::vector<Point>&
    {
        return points_;
    }

    constexpr auto operator[](std::size_t index) const noexcept -> Point
    {
        // non-modifying access with bounds checking on during debug mode only
        assert(index < points_.size());
        return points_[index];
    }

    constexpr auto operator[](std::size_t index) noexcept -> Point&
    {
        // modifying access with bounds checking on during debug mode only
        assert(index < points_.size());
        return points_[index];
    }

    constexpr auto size() const noexcept -> std::size_t
    {
        return points_.size();
    }

private:
    std::vector<Point> points_ {};
};

template <std::floating_point FP, std::size_t NDIM>
constexpr auto worldlines_from_positions(
    const std::vector<coord::Cartesian<FP, NDIM>>& points,
    std::size_t n_timeslices
) noexcept -> std::vector<Worldline<FP, NDIM>>
{
    auto worldlines = std::vector<Worldline<FP, NDIM>> {};
    worldlines.reserve(n_timeslices);

    for (std::size_t i_tslice {0}; i_tslice < n_timeslices; ++i_tslice) {
        worldlines.push_back(Worldline<FP, NDIM> {points});
    }

    return worldlines;
}

}  // namespace worldline
