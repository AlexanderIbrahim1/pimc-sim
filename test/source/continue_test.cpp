#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "simulation/continue.hpp"

TEST_CASE("basic continue file manager", "[ContinueFileManager]")
{
    const auto cfm_impl = impl_continue_sim::ContinueFileManagerImpl_ {};
    const auto i_block = std::size_t {10};
    const auto equilibration_state = true;

    auto continue_stream = std::stringstream {};
    cfm_impl.serialize(continue_stream, {i_block, equilibration_state});
    const auto recovered = cfm_impl.deserialize(continue_stream);

    REQUIRE(recovered.most_recent_block_index == i_block);
    REQUIRE(recovered.is_equilibration_complete == equilibration_state);
}
