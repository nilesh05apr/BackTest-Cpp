cmake_minimum_required(VERSION 3.10)
project(StreamingSupportResistance)

add_executable(StreamingSupportResistance
    main.cpp
)
target_link_libraries(StreamingSupportResistance PRIVATE backtest)
target_include_directories(StreamingSupportResistance PRIVATE

    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Examples/SupportResistance
)