# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(evalTests LANGUAGES CXX)

# ---- Tests ----

add_executable(eval_test source/eval_test.cpp)
target_link_libraries(eval_test PRIVATE eval_lib)
target_compile_features(eval_test PRIVATE cxx_std_17)

add_test(NAME eval_test COMMAND eval_test)

# ---- End-of-file commands ----

add_folders(Test)
