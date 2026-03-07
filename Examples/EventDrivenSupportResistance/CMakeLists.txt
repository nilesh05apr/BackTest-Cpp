add_executable(EventDrivenSupportResistance main.cpp)
target_link_libraries(EventDrivenSupportResistance PRIVATE backtest)
target_include_directories(EventDrivenSupportResistance PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
