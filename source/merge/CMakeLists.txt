set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(Protobuf_INCLUDE_DIR "/usr/local/include")
set(Protobuf_LIBRARY "/usr/local/lib64")
find_package(Protobuf REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/source/external/include)
link_directories(${Protobuf_LIBRARIES})

file(GLOB PROTO_FILES ${CMAKE_SOURCE_DIR}/source/proto/*.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(
    merge_lib OBJECT
    merger.cpp ${PROTO_SRCS}
)

target_include_directories(merge_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${Protobuf_INCLUDE_DIRS})
target_link_libraries(merge_lib protobuf OpenMP::OpenMP_CXX)

add_executable(merge_exe main.cpp)
set_property(TARGET merge_exe PROPERTY OUTPUT_NAME merge)

target_link_libraries(merge_exe merge_lib cnpy z)