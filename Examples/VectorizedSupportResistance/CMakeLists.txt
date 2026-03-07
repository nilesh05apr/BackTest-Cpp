add_executable(VectorizedSupportResistance main.cpp)
target_link_libraries(VectorizedSupportResistance PRIVATE backtest)
target_include_directories(VectorizedSupportResistance PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
