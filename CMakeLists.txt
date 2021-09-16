# CMAKE version
cmake_minimum_required(VERSION 3.8)

# Macros
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-m64 -g -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --gcc-toolchain=/home/gihyeon_lee/OpenSources/GCC")
add_definitions("-Wall -std=c++14")

# Macro
macro(include_module_directory)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

# Headers
include_module_directory()

# Source files
set(SOURCE_FILES
    test.cpp
)

# Executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
add_executable(Main test.cpp)
