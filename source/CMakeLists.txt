cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Get Torch ----

find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# ---- Declare executable and alias name ----

add_executable(
    pimc-sim_exe
    main.cpp
)

add_executable(
    pimc-sim::exe
    ALIAS pimc-sim_exe
)

# ---- Modify executable properties ----

set_property(
    TARGET pimc-sim_exe
    PROPERTY OUTPUT_NAME pimc-sim
)

target_compile_features(
    pimc-sim_exe
    PRIVATE cxx_std_20
)

target_include_directories(
    pimc-sim_exe
    ${warning_guard}
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/extern>"
    PUBLIC "${TORCH_INCLUDE_DIRS}"
)

target_link_libraries(
    pimc-sim_exe
    PRIVATE "${TORCH_LIBRARIES}"
)
