add_library(
    utils_lib OBJECT
    utils.cpp
)

target_link_libraries(utils_lib PRIVATE libcnpy.a OpenMP::OpenMP_CXX)