# CMAKE version
cmake_minimum_required(VERSION 3.8)

# Macros
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-m64")
add_definitions("-Wall -std=c++14")

# Source files
set(SOURCE_FILES
    test.cpp
)

# Executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
add_executable(Main test.cpp)
