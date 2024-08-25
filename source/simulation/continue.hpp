#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include <../extern/tomlplusplus/toml.hpp>

namespace sim
{

constexpr inline auto DEFAULT_CONTINUE_FILE_NAME = std::string_view {"continue.toml"};

constexpr auto continue_file_header() noexcept -> std::string
{
    using namespace std::literals::string_literals;
    return "# this file contains the information needed to continue a simulation\n"s;
}

class _ContinueFileManagerImpl
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
        toml_stream << continue_file_header();

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

class ContinueFileManager
{
public:
    ContinueFileManager(
        const std::filesystem::path& continue_dirpath,
        std::string_view continue_filename = DEFAULT_CONTINUE_FILE_NAME
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
        auto in_stream = std::ifstream {continue_filepath_};
        if (!in_stream.is_open()) {
            auto err_msg = std::stringstream {};
            err_msg << "Error: Unable to open file: '" << continue_filepath_ << "'\n";
            throw std::ios_base::failure {err_msg.str()};
        }

        return impl_.parse_block_index(in_stream);
    }

    void serialize_block_index(std::size_t i_block) const
    {
        auto out_stream = std::ofstream {continue_filepath_, std::ios::out};
        if (!out_stream.is_open()) {
            auto err_msg = std::stringstream {};
            err_msg << "Error: Unable to open file: '" << continue_filepath_ << "'\n";
            throw std::ios_base::failure {err_msg.str()};
        }

        return impl_.serialize_block_index(out_stream, i_block);
    }

private:
    std::filesystem::path continue_filepath_;
    _ContinueFileManagerImpl impl_;
};

}  // namespace sim
