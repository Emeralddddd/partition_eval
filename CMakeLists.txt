cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)
include(ExternalProject)

project(
    eval
    VERSION 0.1.0
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Declare library ----
include_directories(${CMAKE_SOURCE_DIR}/source/external/include/)
include_directories(${CMAKE_BINARY_DIR}/_deps/cnpy-src)
link_directories(${CMAKE_BINARY_DIR}/_deps/cnpy-build)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)
find_package(OpenMP REQUIRED)

include(FetchContent)
FetchContent_Declare(
  cnpy
  GIT_REPOSITORY https://github.com/rogersce/cnpy.git
  GIT_TAG master
)
FetchContent_MakeAvailable(cnpy)

# set(cnpy_static  ${CMAKE_BINARY_DIR}/_deps/cnpy_build)

add_subdirectory(source/merge)
add_subdirectory(source/eval)
add_subdirectory(source/embed_server)
add_subdirectory(source/utils)

# # ---- Declare executable ----

# if (OPENMP_FOUND)
#     target_compile_options(eval_exe PUBLIC ${OpenMP_CXX_FLAGS})
# endif()

# # ---- Install rules ----

# if(NOT CMAKE_SKIP_INSTALL_RULES)
#   include(cmake/install-rules.cmake)
# endif()

# # ---- Developer mode ----

# if(NOT eval_DEVELOPER_MODE)
#   return()
# elseif(NOT PROJECT_IS_TOP_LEVEL)
#   message(
#       AUTHOR_WARNING
#       "Developer mode is intended for developers of eval"
#   )
# endif()

# include(cmake/dev-mode.cmake)
