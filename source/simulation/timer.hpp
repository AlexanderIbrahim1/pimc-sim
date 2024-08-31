#pragma once

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <sstream>
#include <string>
#include <string_view>

#include <common/writers/triple_value_writer.hpp>

/*
NOTES:
  - learned I can't create namespace aliases for classes
    - standard just doesn't allow it, but I couldn't find out why
    - best answer I saw is that the committee foresaw that it would cause severe parsing issues
  - anonymous namespaces are discouraged in header files
*/

namespace impl_timer_sim
{

constexpr inline auto DEFAULT_TIMER_FILENAME = std::string_view {"timer.dat"};

inline auto timer_file_header() -> std::string
{
    auto message = std::stringstream {};

    message << "# this file contains information about the duration of time spent on each block\n";
    message << "# there are four space-separated columns\n";
    message << "# the first is the number label for the block\n";
    message << "# the next three represent the duration split, split into seconds, milliseconds, and microseconds\n";
    message << "# \n";
    message << "# for example, if a line looks like\n";
    message << "#   00205         12        345        678\n";
    message << "# this means block 205 took 12.345678 microseconds to perform\n";

    return message.str();
}

}  // namespace impl_timer_sim


namespace sim
{

struct Duration {
    std::size_t seconds;
    std::size_t milliseconds;
    std::size_t microseconds;
};

class Timer {
public:
    using clock = std::chrono::steady_clock;

    Timer()
        : start_time_point_ {clock::now()}
    {}

    void start() {
        start_time_point_ = clock::now();
    }

    auto duration_since_last_start() const -> Duration
    {
        namespace chr = std::chrono;

        const auto now = clock::now();
        auto duration = now - start_time_point_;

        const auto seconds = chr::duration_cast<chr::seconds>(duration);
        duration -= seconds;

        const auto milliseconds = chr::duration_cast<chr::milliseconds>(duration);
        duration -= milliseconds;

        const auto microseconds = chr::duration_cast<chr::microseconds>(duration);

        return Duration{
            static_cast<std::size_t>(seconds.count()),
            static_cast<std::size_t>(milliseconds.count()),
            static_cast<std::size_t>(microseconds.count())
        };
    }

private:
    clock::time_point start_time_point_;
};


inline auto default_timer_writer(const std::filesystem::path& output_dirpath)
    -> common::writers::TripleValueBlockWriter<std::size_t, std::size_t, std::size_t>
{
    const auto filepath = output_dirpath / impl_timer_sim::DEFAULT_TIMER_FILENAME;
    const auto header = impl_timer_sim::timer_file_header();

    return common::writers::TripleValueBlockWriter<std::size_t, std::size_t, std::size_t> {filepath, header};
}

}  // namespace sim
