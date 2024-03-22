#pragma once

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <sstream>
#include <string>

#include <worldline/writers/worldline_writer.hpp>

namespace worldline
{

template <std::floating_point FP, std::size_t NDIM>
auto delete_worldlines_file(
    const WorldlineWriter<FP, NDIM>& worldline_writer,
    std::size_t i_block,
    std::size_t n_most_recent
) -> bool
{
    /*
    Finds the worldline file at block `i_block - n_most_recent`, creates the corresponding filepath,
    and deletes the file. This function saves the user from having to do any index fiddling, and
    just pass in `n_most_recent` as the number of most recent worldline files they want to exist.

    If the calculation for the worldline block turns out to be negative, or that worldline block does
    not exist, then nothing is done. The function returns a boolean indicating if a deletion occured.
    */

    if (n_most_recent > i_block) {
        return false;
    }

    const auto i_block_to_delete = i_block - n_most_recent;
    const auto filepath = worldline_writer.output_filepath(i_block_to_delete);

    // NOTE: std::filesystem::remove already returns `false` if the file did not exist
    return std::filesystem::remove(filepath);
}

}  // namespace worldline
