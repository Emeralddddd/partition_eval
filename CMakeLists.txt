cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)
include(ExternalProject)

project(
    eval
    VERSION 0.1.0
    DESCRIPTION "eval partition results"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    eval_lib OBJECT
    source/lib.cpp source/partition.cpp source/eval.cpp
)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
find_package(OpenMP REQUIRED)

target_include_directories(
    eval_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)
target_compile_features(eval_lib PUBLIC cxx_std_17)

# ---- Declare executable ----

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(eval_exe source/main.cpp)
add_executable(eval::exe ALIAS eval_exe)

set_property(TARGET eval_exe PROPERTY OUTPUT_NAME eval)

target_compile_features(eval_exe PRIVATE cxx_std_17)

target_link_libraries(eval_exe PRIVATE eval_lib)
target_link_libraries(eval_exe PRIVATE cnpy z)
target_link_libraries(eval_exe PUBLIC OpenMP::OpenMP_CXX)
if (OPENMP_FOUND)
    target_compile_options(eval_exe PUBLIC ${OpenMP_CXX_FLAGS})
endif()

add_subdirectory(source/query)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT eval_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of eval"
  )
endif()

include(cmake/dev-mode.cmake)
