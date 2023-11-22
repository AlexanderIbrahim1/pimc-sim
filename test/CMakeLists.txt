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

add_executable(mymath_test source/mymath_test.cpp)
target_compile_features(mymath_test PRIVATE cxx_std_20)
target_include_directories(mymath_test PRIVATE "${SOURCE_FILES_DIR}")
target_link_libraries(
    mymath_test
    PRIVATE Catch2::Catch2WithMain
)

catch_discover_tests(mymath_test)

# ---- End-of-file commands ----

add_folders(Test)
