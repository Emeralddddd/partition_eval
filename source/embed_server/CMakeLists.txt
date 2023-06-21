include(FetchContent)
set(ABSL_ENABLE_INSTALL ON)
FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.56.0  # e.g v1.28.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

add_library(embed_server_lib OBJECT inner_embed.cpp)

target_link_libraries(embed_server_lib PRIVATE OpenMP::OpenMP_CXX grpc++)

add_executable(embed_server_exe main.cpp)
set_property(TARGET embed_server_exe PROPERTY OUTPUT_NAME embed_server)

target_link_libraries(embed_server_exe embed_server_lib)