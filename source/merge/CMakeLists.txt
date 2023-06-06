include_directories(${CMAKE_SOURCE_DIR}/source/external/include)

add_executable(merge_exe main.cpp)
add_executable(merge::exe ALIAS merge_exe)

set_property(TARGET merge_exe PROPERTY OUTPUT_NAME merge)

# target_link_libraries(query_exe hiredis)
# target_link_libraries(query_exe OpenMP::OpenMP_CXX)
target_link_libraries(query_exe cnpy z)
# target_link_libraries(query_exe /usr/local/lib64/libhiredis.a)
