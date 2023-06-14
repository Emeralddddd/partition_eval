include_directories(${CMAKE_SOURCE_DIR}/source/external/include)

add_library(
    merge_lib OBJECT
    merger.cpp
)
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(Protobuf_INCLUDE_DIR "/usr/local/protobuf/include")
set(Protobuf_LIBRARY "/usr/local/protobuf/lib")
find_package(Protobuf REQUIRED)
set(PROTO_FILES ../proto/partial_result.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_executable(merge_exe main.cpp ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(merge::exe ALIAS merge_exe)

set_property(TARGET merge_exe PROPERTY OUTPUT_NAME merge)

target_link_libraries(merge_exe OpenMP::OpenMP_CXX)
target_link_libraries(merge_exe cnpy z)
target_link_libraries(merge_exe merge_lib)
target_link_libraries(merge_exe /usr/local/protobuf/lib/libprotobuf.so /usr/local/protobuf/lib/libprotobuf.a)
target_include_directories(merge_exe PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(merge_exe PRIVATE ${Protobuf_INCLUDE_DIRS})
