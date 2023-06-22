include(FetchContent)
set(ABSL_ENABLE_INSTALL ON)
FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.56.0  # e.g v1.28.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

file(GLOB PROTO_FILES ${CMAKE_SOURCE_DIR}/source/proto/embed_server.proto)

file(GLOB PROTO_SRCS ${CMAKE_CURRENT_BINARY_DIR}/*.grpc.pb.cc)
file(GLOB GRPC_SRCS ${CMAKE_CURRENT_BINARY_DIR}/*.pb.cc)

add_library(embed_server_lib OBJECT inner_embed.cpp embed_server.cpp embed_client.cpp infer_server.cpp ${PROTO_SRCS} ${GRPC_SRCS})
message(Status ${grpc_SOURCE_DIR}/include)
target_include_directories(embed_server_lib PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${grpc_SOURCE_DIR}/include 
  ${grpc_SOURCE_DIR}/third_party/abseil-cpp
  ${grpc_SOURCE_DIR}/third_party/protobuf/src)
target_link_libraries(embed_server_lib PRIVATE OpenMP::OpenMP_CXX grpc++)

add_executable(embed_server_exe main.cpp)
set_property(TARGET embed_server_exe PROPERTY OUTPUT_NAME embed_server)

add_executable(embed_client_exe client_main.cpp)
set_property(TARGET embed_client_exe PROPERTY OUTPUT_NAME embed_client)

target_link_libraries(embed_server_exe embed_server_lib)
target_link_libraries(embed_client_exe embed_server_lib)