#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>

#include "test_utils.hpp"

namespace test_utils
{

auto resolve_project_path(const std::filesystem::path& rel_filepath) -> std::filesystem::path
{
    // NOTE: this is probably a bit unsafe? What if somebody puts a particular directory in the way?
    auto base_dirpath = std::filesystem::current_path();
    while (base_dirpath.has_parent_path()) {
        const auto combine_path = base_dirpath / rel_filepath;
        if (std::filesystem::exists(combine_path)) {
            return combine_path;
        }

        base_dirpath = base_dirpath.parent_path();
    }

    auto err_msg = std::stringstream {};
    err_msg << "Error: relative path not resolved: '" << rel_filepath.string() << "'\n";
    throw std::runtime_error(err_msg.str());
}

}  // namespace test_utils
