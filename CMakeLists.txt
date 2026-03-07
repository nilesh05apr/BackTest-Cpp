cmake_minimum_required(VERSION 3.10)

project(BackTest-Cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only backtest core
add_library(backtest INTERFACE)
target_include_directories(backtest INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Enable CTest
enable_testing()

# Build example projects
add_subdirectory(Examples)

# Unit and integration tests
add_subdirectory(Tests)
