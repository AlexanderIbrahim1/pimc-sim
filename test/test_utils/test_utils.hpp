#pragma once

#include <filesystem>

namespace test_utils
{

auto resolve_project_path(const std::filesystem::path& rel_filepath) -> std::filesystem::path;

}  // namespace test_utils
