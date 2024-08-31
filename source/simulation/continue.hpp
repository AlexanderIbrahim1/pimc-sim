#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include <common/io_utils.hpp>

#include <../extern/tomlplusplus/toml.hpp>

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
    auto parse_block_index(std::istream& toml_stream) const -> std::size_t
    {
        const auto table = toml::parse(toml_stream);

        const auto maybe_value = table[block_index_name_].value<std::size_t>();
        if (!maybe_value) {
            auto err_msg = std::stringstream {};
            err_msg << "Failed to parse '" << block_index_name_ << "' from the toml stream.\n";
            throw std::runtime_error {err_msg.str()};
        }

        return *maybe_value;
    }

    void serialize_block_index(std::ostream& toml_stream, std::size_t i_block) const
    {
        toml_stream << continue_file_header_();

        // tomlplusplus requires that "Integral value initializers must be losslessly convertible to int64_t"
        // - we already know that `i_block` must be positive, so we don't really lose information here
        const auto output_i_block = static_cast<std::int64_t>(i_block);

        const auto table = toml::table {
            {block_index_name_, output_i_block}
        };

        toml_stream << table;
    }

private:
    std::string_view block_index_name_ {"most_recent_block_index"};
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

    auto parse_block_index() const -> std::size_t
    {
        auto in_stream = common_utils::open_input_filestream_checked(continue_filepath_);
        return impl_.parse_block_index(in_stream);
    }

    void serialize_block_index(std::size_t i_block) const
    {
        auto out_stream = common_utils::open_output_filestream_checked(continue_filepath_);
        return impl_.serialize_block_index(out_stream, i_block);
    }

private:
    std::filesystem::path continue_filepath_;
    impl_continue_sim::ContinueFileManagerImpl_ impl_;
};

}  // namespace sim
