include_directories(${CMAKE_SOURCE_DIR}/source/external/include)

add_library(
    merge_lib OBJECT
    merger.cpp
)

add_executable(merge_exe main.cpp)
add_executable(merge::exe ALIAS merge_exe)

set_property(TARGET merge_exe PROPERTY OUTPUT_NAME merge)
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
# target_link_libraries(query_exe hiredis)
target_link_libraries(merge_exe OpenMP::OpenMP_CXX)
target_link_libraries(merge_exe cnpy z)
target_link_libraries(merge_exe merge_lib)
# target_link_libraries(query_exe /usr/local/lib64/libhiredis.a)