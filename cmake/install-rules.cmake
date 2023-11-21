install(
    TARGETS pimc-sim_exe
    RUNTIME COMPONENT pimc-sim_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
