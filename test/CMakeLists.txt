# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(pimc-simTests LANGUAGES CXX)

# ---- Tests ----

add_executable(pimc-sim_test source/pimc-sim_test.cpp)
target_compile_features(pimc-sim_test PRIVATE cxx_std_20)

add_test(NAME pimc-sim_test COMMAND pimc-sim_test)

# ---- End-of-file commands ----

add_folders(Test)
