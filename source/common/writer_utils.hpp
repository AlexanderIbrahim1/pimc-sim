#pragma once

#include <istream>
#include <string>

namespace common
{

namespace writers
{

constexpr auto DEFAULT_WRITER_BLOCK_INDEX_PADDING = int {5};
constexpr auto DEFAULT_WRITER_FLOATING_POINT_PRECISION = int {8};
constexpr auto DEFAULT_WRITER_INTEGER_PADDING = int {8};
constexpr auto DEFAULT_TEMPORARY_SUFFIX = std::string {"_TEMPORARY"};
constexpr auto DEFAULT_MULTICOLUMN_SPACES = std::string {"   "};
constexpr auto DEFAULT_SPACING = std::size_t {3};

inline void skip_lines_starting_with(std::istream& stream, char c)
{
    auto line = std::string {};
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }

        if (line[0] != c) {
            stream.putback('\n');
            for (std::size_t i {line.size()}; i > 0; --i) {
                stream.putback(line[i - 1]);
            }
            break;
        }
    }
}

}  // namespace writers

}  // namespace common
