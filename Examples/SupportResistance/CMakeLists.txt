add_executable(SupportResistance main.cpp SupportResistance.cpp)

target_link_libraries(SupportResistance PRIVATE core)

target_include_directories(SupportResistance PRIVATE ${CMAKE_SOURCE_DIR}/include)
