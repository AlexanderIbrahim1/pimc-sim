#pragma once

#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>


namespace rng
{

enum class RandomSeedFlag
{
    RANDOM,
    TIME_SINCE_EPOCH
};

constexpr auto DEFAULT_PRNG_STATE_FILENAME = std::string_view {"prng.state"};

inline auto default_prng_state_filepath(const std::filesystem::path& output_dirpath) -> std::filesystem::path {
    return output_dirpath / DEFAULT_PRNG_STATE_FILENAME;
}

inline void save_prng_state(const std::mt19937& prng, std::ofstream& out_stream) {
    out_stream << prng;
}

inline void load_prng_state(std::mt19937& prng, std::ifstream& in_stream) {
    in_stream >> prng;
}

inline void save_prng_state(const std::mt19937& prng, const std::filesystem::path& prng_state_filepath) {
    auto out_stream = std::ofstream {prng_state_filepath, std::ios::out};

    if (!out_stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to open the PRNG state file for saving: '" << prng_state_filepath << "'\n";
        throw std::runtime_error {err_msg.str()};
    }

    save_prng_state(prng, out_stream);
}

inline void load_prng_state(std::mt19937& prng, const std::filesystem::path& prng_state_filepath) {
    auto in_stream = std::ifstream {prng_state_filepath, std::ios::in};

    if (!in_stream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to open the PRNG state file for loading: '" << prng_state_filepath << "'\n";
        throw std::runtime_error {err_msg.str()};
    }

    load_prng_state(prng, in_stream);
}

}  // namespace rng
