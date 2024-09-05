cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Get Torch ----

find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# ---- Declare executable and alias name for main ----

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
    SYSTEM PRIVATE "${TORCH_INCLUDE_DIRS}"
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/extern>"
)

target_link_libraries(
    pimc-sim_exe
    PRIVATE "${TORCH_LIBRARIES}"
)

# ---- Declare executable and alias name for evaluate_worldline ----

add_executable(
    evaluate-worldline_exe
    evaluate_worldline.cpp
)

add_executable(
    evaluate-worldline::exe
    ALIAS evaluate-worldline_exe
)

# ---- Modify executable properties ----

set_property(
    TARGET evaluate-worldline_exe
    PROPERTY OUTPUT_NAME evaluate-worldline
)

target_compile_features(
    evaluate-worldline_exe
    PRIVATE cxx_std_20
)

target_include_directories(
    evaluate-worldline_exe
    ${warning_guard}
    SYSTEM PRIVATE "${TORCH_INCLUDE_DIRS}"
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/extern>"
)

target_link_libraries(
    evaluate-worldline_exe
    PRIVATE "${TORCH_LIBRARIES}"
)
