#pragma once

#include <istream>
#include <string>

namespace common_utils
{

namespace writer_utils
{

constexpr auto DEFAULT_WRITER_BLOCK_INDEX_PADDING = int {5};
constexpr auto DEFAULT_WRITER_SINGLE_VALUE_PRECISION = int {8};
constexpr auto DEFAULT_WRITER_INTEGER_PADDING = int {8};

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

}  // namespace writer_utils

}  // namespace common_utils
