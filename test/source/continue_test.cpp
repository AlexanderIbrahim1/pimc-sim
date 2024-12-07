#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "simulation/continue.hpp"

TEST_CASE("basic continue file manager", "[ContinueFileManager]")
{
    const auto cfm_impl = impl_continue_sim::ContinueFileManagerImpl_ {};
    const auto i_block = std::size_t {10};
    const auto i_worldline = std::size_t {8};
    const auto worldline_saved_state = true;
    const auto equilibration_state = true;

    auto continue_stream = std::stringstream {};
    cfm_impl.serialize(continue_stream, {i_block, i_worldline, worldline_saved_state, equilibration_state});
    const auto recovered = cfm_impl.deserialize(continue_stream);

    REQUIRE(recovered.most_recent_block_index == i_block);
    REQUIRE(recovered.most_recent_saved_worldline_index == i_worldline);
    REQUIRE(recovered.is_at_least_one_worldline_index_saved == worldline_saved_state);
    REQUIRE(recovered.is_equilibration_complete == equilibration_state);
}
