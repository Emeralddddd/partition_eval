add_library(embed_server_lib OBJECT inner_embed.cpp)

target_link_libraries(embed_server_lib PRIVATE OpenMP::OpenMP_CXX)

add_executable(embed_server_exe main.cpp)
set_property(TARGET embed_server_exe PROPERTY OUTPUT_NAME embed_server)

target_link_libraries(embed_server_exe embed_server_lib)