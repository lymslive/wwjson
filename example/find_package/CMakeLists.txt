# CMakeLists.txt for find_package usage example
# This demonstrates how to use wwjson after system-wide installation
cmake_minimum_required(VERSION 3.15)
project(wwjson_find_package_example LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find wwjson package (installed system-wide or via CMAKE_PREFIX_PATH)
find_package(wwjson REQUIRED)

# Print a single debug line showing where the wwjson package was found/installed
if(DEFINED wwjson_DIR)
    set(_WWJSON_INSTALL_LOCATION ${wwjson_DIR})
elseif(TARGET wwjson::wwjson)
    get_target_property(_wwjson_includes wwjson::wwjson INTERFACE_INCLUDE_DIRECTORIES)
    if(_wwjson_includes)
        list(GET _wwjson_includes 0 _WWJSON_INSTALL_LOCATION)
    else()
        set(_WWJSON_INSTALL_LOCATION "<interface dirs unknown>")
    endif()
else()
    set(_WWJSON_INSTALL_LOCATION "<unknown>")
endif()

message(STATUS "wwjson found installed at: ${_WWJSON_INSTALL_LOCATION}")

# Example 1: struct_to_json
add_executable(example_struct_to_json
    ${CMAKE_CURRENT_SOURCE_DIR}/../struct_to_json.cpp
)
target_link_libraries(example_struct_to_json PRIVATE wwjson::wwjson)

# Example 2: estimate_size
add_executable(example_estimate_size
    ${CMAKE_CURRENT_SOURCE_DIR}/../estimate_size.cpp
)
target_link_libraries(example_estimate_size PRIVATE wwjson::wwjson)

# Example 3: hex_json
add_executable(example_hex_json
    ${CMAKE_CURRENT_SOURCE_DIR}/../hex_json.cpp
)
target_link_libraries(example_hex_json PRIVATE wwjson::wwjson)

# Custom target to run all examples
add_custom_target(run_examples
    COMMAND ${CMAKE_COMMAND} -E echo "Running example_struct_to_json..."
    COMMAND example_struct_to_json
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Running example_estimate_size..."
    COMMAND example_estimate_size
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Running example_hex_json..."
    COMMAND example_hex_json
    DEPENDS example_struct_to_json example_estimate_size example_hex_json
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all example programs"
)
