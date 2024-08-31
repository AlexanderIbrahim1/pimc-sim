#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include <common/io_utils.hpp>
#include <common/toml_utils.hpp>

#include <../extern/tomlplusplus/toml.hpp>

namespace sim
{

struct SimulationContinueInfo
{
    std::size_t most_recent_block_index;
    bool is_equilibration_complete;
};

}  // namespace sim


namespace impl_continue_sim
{

constexpr auto DEFAULT_CONTINUE_FILENAME = std::string_view {"continue.toml"};

constexpr auto continue_file_header_() noexcept -> std::string
{
    using namespace std::literals::string_literals;
    return "# this file contains the information needed to continue a simulation\n"s;
}

class ContinueFileManagerImpl_
{
public:
    auto deserialize(std::istream& toml_stream) const -> sim::SimulationContinueInfo
    {
        using common_utils::cast_toml_to;

        const auto table = toml::parse(toml_stream);

        const auto most_recent_block_index = cast_toml_to<std::size_t>(table, most_recent_block_index_name_);
        const auto is_equilibration_complete = cast_toml_to<bool>(table, is_equilibration_complete_name_);

        return sim::SimulationContinueInfo {
            most_recent_block_index,
            is_equilibration_complete
        };
    }

    void serialize(std::ostream& toml_stream, const sim::SimulationContinueInfo& continue_info) const
    {
        toml_stream << continue_file_header_();

        // tomlplusplus requires that "Integral value initializers must be losslessly convertible to int64_t"
        // - we already know that `i_block` must be positive, so we don't really lose information here
        const auto most_recent_block_index = static_cast<std::int64_t>(continue_info.most_recent_block_index);

        const auto table = toml::table {
            {most_recent_block_index_name_, most_recent_block_index},
            {is_equilibration_complete_name_, continue_info.is_equilibration_complete}
        };

        toml_stream << table;
    }

private:
    std::string_view most_recent_block_index_name_ {"most_recent_block_index"};
    std::string_view is_equilibration_complete_name_ {"is_equilibration_complete"};
};

}  // namespace impl_continue_sim


namespace sim
{

class ContinueFileManager
{
public:
    ContinueFileManager(
        const std::filesystem::path& continue_dirpath,
        std::string_view continue_filename = impl_continue_sim::DEFAULT_CONTINUE_FILENAME
    )
        : continue_filepath_ {continue_dirpath / continue_filename}
    {}

    auto file_exists() const noexcept -> bool
    {
        return std::filesystem::exists(continue_filepath_);
    }

    auto is_continued() const noexcept -> bool
    {
        // for now, the continue file's existence is the only thing that determines if the simulation is being continued
        return file_exists();
    }

    void deserialize()
    {
        auto in_stream = common_utils::open_input_filestream_checked(continue_filepath_);
        info_ = impl_.deserialize(in_stream);
    }

    void serialize()
    {
        auto out_stream = common_utils::open_output_filestream_checked(continue_filepath_);
        impl_.serialize(out_stream, info_);
    }

    constexpr auto get_info() const -> sim::SimulationContinueInfo
    {
        // getting/setting and deserializing/serializing are separated so the toml file doesn't have to be parsed
        // every time we want some of its info
        return info_;
    }

    constexpr void set_info(const sim::SimulationContinueInfo& continue_info)
    {
        // getting/setting and deserializing/serializing are separated so the toml file doesn't have to be parsed
        // every time we want some of its info
        info_ = continue_info;
    }

    void set_info_and_serialize(const sim::SimulationContinueInfo& continue_info)
    {
        set_info(continue_info);
        serialize();
    }

private:
    std::filesystem::path continue_filepath_;
    impl_continue_sim::ContinueFileManagerImpl_ impl_;
    sim::SimulationContinueInfo info_;
};

}  // namespace sim
