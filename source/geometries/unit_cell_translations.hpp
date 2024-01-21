#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <format>
#include <utility>

#include <geometries/geom_utils.hpp>

namespace geom
{

template <std::size_t NDIM>
class UnitCellTranslations
{
public:
    template <typename... VarCoords>
    UnitCellTranslations(VarCoords... coords)
        : translations_ {(coords)...}
    {
        static_assert(sizeof...(coords) == NDIM);
        geom_utils::check_unit_cell_translations_are_positive(translations_);
    }

    constexpr auto translations() const noexcept -> const std::array<std::size_t, NDIM>&
    {
        return translations_;
    }

private:
    std::array<std::size_t, NDIM> translations_ {};
};

template <std::size_t NDIM>
constexpr auto n_total_boxes(const UnitCellTranslations<NDIM>& translations) noexcept -> std::size_t
{
    auto n_boxes = std::size_t {1};
    for (auto trans : translations.translations()) {
        n_boxes *= trans;
    }
    return n_boxes;
}

template <std::size_t NDIM>
class UnitCellIncrementer
{
public:
    explicit UnitCellIncrementer(UnitCellTranslations<NDIM> translations)
        : translations_ {std::move(translations)}
        , unit_cell_index_ {}
    {}

    void increment() noexcept
    {
        const auto translations = translations_.translations();
        auto remainder = std::size_t {1};

        for (std::size_t i {0}; i < NDIM; ++i) {
            const auto new_index = (unit_cell_index_[i] + remainder) % translations[i];
            remainder = (unit_cell_index_[i] + remainder) / translations[i];

            unit_cell_index_[i] = new_index;
        }
    }

    constexpr auto indices() const noexcept -> const std::array<std::size_t, NDIM>&
    {
        return unit_cell_index_;
    }

private:
    UnitCellTranslations<NDIM> translations_ {};
    std::array<std::size_t, NDIM> unit_cell_index_ {};
};

}  // namespace geom
