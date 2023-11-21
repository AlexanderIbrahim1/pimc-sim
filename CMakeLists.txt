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

add_subdirectory(source) # pimc-sim_exe

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
