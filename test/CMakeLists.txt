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

# ---- Get Torch ----

find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# ---- Get access to multiple argument parser ----
include(CMakeParseArguments)

# ---- Tests ----

cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH PIMC_SIM_SOURCE_DIR)
set(SOURCE_FILES_DIR "${PIMC_SIM_SOURCE_DIR}/source")

# TODO: learn how to conditionally include things (not each test requires filesearch.cpp, for example)
# function(add_test_target TARGET_NAME SOURCE_FILE OTHERS)
function(add_test_target)
    set(ONE_VALUE_ARGS TARGET)
    set(MULTI_VALUE_ARGS SOURCES)
    cmake_parse_arguments(add_test_target "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

    add_executable(
        ${add_test_target_TARGET}
        ${add_test_target_SOURCES}
    )

    target_compile_features(
        ${add_test_target_TARGET}
        PRIVATE cxx_std_20
    )

    target_include_directories(
        ${add_test_target_TARGET}
        ${warning_guard}
        PRIVATE "${SOURCE_FILES_DIR}"
        PRIVATE "test_utils"
    )

    target_link_libraries(
        ${add_test_target_TARGET}
        PRIVATE Catch2::Catch2WithMain
    )

    catch_discover_tests(${add_test_target_TARGET})
endfunction()

function(add_test_target_with_torch)
    set(ONE_VALUE_ARGS TARGET)
    set(MULTI_VALUE_ARGS SOURCES)
    cmake_parse_arguments(add_test_target "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

    add_executable(
        ${add_test_target_TARGET}
        ${add_test_target_SOURCES}
    )

    target_compile_features(
        ${add_test_target_TARGET}
        PRIVATE cxx_std_20
    )

    target_include_directories(
        ${add_test_target_TARGET}
        ${warning_guard}
        SYSTEM PRIVATE "${TORCH_INCLUDE_DIRS}"
        PRIVATE "${SOURCE_FILES_DIR}"
        PRIVATE "test_utils"
    )

    target_link_libraries(
        ${add_test_target_TARGET}
        PRIVATE Catch2::Catch2WithMain
        PRIVATE "${TORCH_LIBRARIES}"
    )

    catch_discover_tests(${add_test_target_TARGET})
endfunction()

add_test_target(TARGET cartesian_test SOURCES "source/cartesian_test.cpp")
add_test_target(TARGET measure_test SOURCES "source/measure_test.cpp")
add_test_target(TARGET worldlines_test SOURCES "source/worldlines_test.cpp")
add_test_target(TARGET two_body_test SOURCES "source/two_body_test.cpp")
add_test_target(TARGET unit_cell_test SOURCES "source/unit_cell_test.cpp")
add_test_target(TARGET unit_cell_translations_test SOURCES "source/unit_cell_translations_test.cpp")
add_test_target(TARGET lattce_test SOURCES "source/lattice_test.cpp")
add_test_target(TARGET linear_interp_test SOURCES "source/linear_interp_test.cpp")
add_test_target(TARGET fsh_potential_test SOURCES "source/fsh_potential_test.cpp" "test_utils/test_utils.cpp")
add_test_target(TARGET grid2d_test SOURCES "source/grid2d_test.cpp")
add_test_target(TARGET bisection_level_manager_test SOURCES "source/bisection_level_manager_test.cpp")
add_test_target(TARGET square_adjacency_matrix_test SOURCES "source/square_adjacency_matrix_test.cpp")
add_test_target(TARGET adjuster_test SOURCES "source/adjuster_test.cpp")
add_test_target(TARGET histogram_test SOURCES "source/histogram_test.cpp")
add_test_target(TARGET continue_test SOURCES "source/continue_test.cpp")
add_test_target(TARGET trilinear_interp_test SOURCES "source/trilinear_interp_test.cpp")
add_test_target(TARGET attard_test SOURCES "source/attard_test.cpp")
add_test_target(TARGET composite_handler_test SOURCES "source/composite_handler_test.cpp")
add_test_target(TARGET dispersion_potential_test SOURCES "source/dispersion_potential_test.cpp")
add_test_target(TARGET prng_state_test SOURCES "source/prng_state_test.cpp")
add_test_target(TARGET buffered_writer_test SOURCES "source/buffered_writer_test.cpp" "test_utils/test_utils.cpp")

# compiling the transformer and four_body tests requires torch, which bloats the compile times;
# as a result, I try to run these tests only every once in a while
#   add_test_target_with_torch(TARGET four_body_test SOURCES "source/four_body_test.cpp" "test_utils/test_utils.cpp" "test_utils/test_tensor_utils.cpp")
#   add_test_target_with_torch(TARGET transformer_test SOURCES "source/transformer_test.cpp")

# ---- End-of-file commands ----

add_folders(Test)
