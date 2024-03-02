#pragma once

#include <filesystem>

namespace test_utils
{

auto resolve_project_path(const std::filesystem::path& rel_filepath) -> std::filesystem::path;
void skip_lines_starting_with(std::istream& stream, char c);

}  // namespace test_utils
