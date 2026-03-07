cmake_minimum_required(VERSION 3.10)

# ── Unit tests ────────────────────────────────────────────────────────────────
add_executable(test_unit test_unit.cpp)
target_link_libraries(test_unit PRIVATE backtest)
target_compile_definitions(test_unit PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Data"
)
add_test(NAME test_unit COMMAND test_unit)

# ── Integration tests ─────────────────────────────────────────────────────────
add_executable(test_integration test_integration.cpp)
target_link_libraries(test_integration PRIVATE backtest)
target_compile_definitions(test_integration PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Data"
    EXAMPLES_DATA_DIR="${CMAKE_SOURCE_DIR}/Examples/Data"
)
add_test(NAME test_integration COMMAND test_integration)
