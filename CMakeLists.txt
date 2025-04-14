cmake_minimum_required(VERSION 3.10)

project(BackTest-Cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable optimizations for release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ")
add_compile_options("-Wall" "-Wextra" "-pedantic")

# Source files
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.h)

# Include directories
include_directories(include)

# Add Core source file libraries.
add_library(core STATIC ${SOURCES})

# Executable
add_executable(BackTest-Cpp ${SOURCES} ${HEADERS})

# Recursively build other projects.
add_subdirectory(Examples)
