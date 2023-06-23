add_library(
    eval_lib OBJECT
    partition.cpp eval.cpp
)

target_include_directories(
    eval_lib
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_link_libraries(eval_lib PUBLIC merge_lib)

add_executable(eval_exe main.cpp)
set_property(TARGET eval_exe PROPERTY OUTPUT_NAME eval)
target_link_libraries(eval_exe PRIVATE eval_lib merge_lib utils_lib libcnpy.a z OpenMP::OpenMP_CXX)

