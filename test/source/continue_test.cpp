#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "simulation/continue.hpp"

TEST_CASE("basic continue file manager", "[ContinueFileManager]")
{
    const auto cfm_impl = impl_continue_sim::ContinueFileManagerImpl_ {};
    const auto i_block = std::size_t {10};

    auto continue_stream = std::stringstream {};
    cfm_impl.serialize_block_index(continue_stream, i_block);
    const auto recovered_i_block = cfm_impl.parse_block_index(continue_stream);

    REQUIRE(recovered_i_block == i_block);
}
