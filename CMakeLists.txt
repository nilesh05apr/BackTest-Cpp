cmake_minimum_required(VERSION 3.10)

# Set the project name
project(BackTestCpp)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the main executable (assuming you have a main.cpp in the root directory)
add_executable(BackTestCpp
    src/Engine/BacktestEngine.cpp
    src/DataReader/Data.cpp
    src/Indicators/Indicator.cpp
    src/Strategy/Strategy.cpp
)

# Include directories
target_include_directories(BackTestCpp PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DataReader
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Indicators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Strategy
)

# Add the SupportResistance example
add_executable(SupportResistanceExample
    Examples/SupportResistance/main.cpp
    Examples/SupportResistance/SupportResistance.cpp
    src/Engine/BacktestEngine.cpp
    src/DataReader/Data.cpp
    src/Indicators/Indicator.cpp
    src/Strategy/Strategy.cpp
)

# Include directories for the example
target_include_directories(SupportResistanceExample PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DataReader
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Indicators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Strategy
    ${CMAKE_CURRENT_SOURCE_DIR}/Examples/SupportResistance
)