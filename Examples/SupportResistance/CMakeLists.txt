add_executable(SupportResistance main.cpp)
target_link_libraries(SupportResistance PRIVATE backtest)
target_include_directories(SupportResistance PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
