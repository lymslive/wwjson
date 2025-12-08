# CMakeLists.txt for performance tests
cmake_minimum_required(VERSION 3.15)

# Create performance test executable
add_executable(pfwwjson
    test_data.cpp
    p_builder.cpp
    p_number.cpp
    p_string.cpp
)

# Link with wwjson library
target_link_libraries(pfwwjson PRIVATE wwjson)

# Link with couttast for testing framework
if(TARGET couttast::couttast)
    target_link_libraries(pfwwjson PRIVATE couttast::couttast)
elseif(TARGET couttast)
    target_link_libraries(pfwwjson PRIVATE couttast)
endif()

# Link with yyjson for performance comparison
if(TARGET yyjson::yyjson)
    target_link_libraries(pfwwjson PRIVATE yyjson::yyjson)
elseif(TARGET yyjson)
    target_link_libraries(pfwwjson PRIVATE yyjson)
endif()

# Add custom target for listing performance tests
add_custom_target(list_perf_tests
    COMMAND pfwwjson --List
    DEPENDS pfwwjson
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running wwjson performance tests with --List option"
)
