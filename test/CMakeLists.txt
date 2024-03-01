# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(pimc-simTests LANGUAGES CXX)


# ---- Get access to Catch2 ----

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY git@github.com:catchorg/Catch2.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)
include(Catch)

# ---- Tests ----

cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH PIMC_SIM_SOURCE_DIR)
set(SOURCE_FILES_DIR "${PIMC_SIM_SOURCE_DIR}/source")

# TODO: learn how to conditionally include things (not each test requires filesearch.cpp, for example)
function(add_test_target TARGET_NAME SOURCE_FILE)
    add_executable(
        ${TARGET_NAME}
        ${SOURCE_FILE}
        "test_utils/filesearch.cpp"
    )

    target_compile_features(
        ${TARGET_NAME}
        PRIVATE cxx_std_20
    )

    target_include_directories(
        ${TARGET_NAME}
        ${warning_guard}
        PRIVATE "${SOURCE_FILES_DIR}"
        PRIVATE "test_utils"
    )

    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE Catch2::Catch2WithMain
    )

    catch_discover_tests(${TARGET_NAME})
endfunction()

add_test_target(cartesian_test "source/cartesian_test.cpp")
add_test_target(measure_test "source/measure_test.cpp")
add_test_target(worldline_test "source/worldline_test.cpp")
add_test_target(two_body_test "source/two_body_test.cpp")
add_test_target(unit_cell_test "source/unit_cell_test.cpp")
add_test_target(unit_cell_translations_test "source/unit_cell_translations_test.cpp")
add_test_target(lattce_test "source/lattice_test.cpp")
add_test_target(linear_interp_test "source/linear_interp_test.cpp")
add_test_target(fsh_potential_test "source/fsh_potential_test.cpp")
add_test_target(grid2d_test "source/grid2d_test.cpp")
add_test_target(bisection_level_manager_test "source/bisection_level_manager_test.cpp")
add_test_target(square_adjacency_matrix_test "source/square_adjacency_matrix_test.cpp")
add_test_target(adjuster_test "source/adjuster_test.cpp")
add_test_target(histogram_test "source/histogram_test.cpp")


# ---- End-of-file commands ----

add_folders(Test)
