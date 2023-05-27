include_directories(/usr/local/include/hiredis)
include_directories(/usr/local/include/hiredis/adapters)
include_directories(${CMAKE_SOURCE_DIR}/source/external/include)

add_executable(query_exe main.cpp)
add_executable(query::exe ALIAS query_exe)

set_property(TARGET query_exe PROPERTY OUTPUT_NAME query)

target_link_libraries(query_exe hiredis)
target_link_libraries(query_exe OpenMP::OpenMP_CXX)
target_link_libraries(query_exe cnpy z)
# target_link_libraries(query_exe /usr/local/lib64/libhiredis.a)
