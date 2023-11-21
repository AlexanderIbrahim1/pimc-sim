cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    pimc-sim
    VERSION 0.1.0
    DESCRIPTION "Project for performing PIMC simulations of solid parahydrogen."
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    pimc-sim_lib OBJECT
    source/lib.cpp
)

target_include_directories(
    pimc-sim_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(pimc-sim_lib PUBLIC cxx_std_20)

# ---- Declare executable ----

add_executable(pimc-sim_exe source/main.cpp)
add_executable(pimc-sim::exe ALIAS pimc-sim_exe)

set_property(TARGET pimc-sim_exe PROPERTY OUTPUT_NAME pimc-sim)

target_compile_features(pimc-sim_exe PRIVATE cxx_std_20)

target_link_libraries(pimc-sim_exe PRIVATE pimc-sim_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT pimc-sim_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of pimc-sim"
  )
endif()

include(cmake/dev-mode.cmake)
