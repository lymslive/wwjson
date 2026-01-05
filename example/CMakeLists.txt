# CMakeLists.txt for example programs
cmake_minimum_required(VERSION 3.15)

message(STATUS "Building example programs")

# Example 1: Multi-level structure to JSON
add_executable(example_struct_to_json
    struct_to_json.cpp
)

# Add compile definitions for examples
target_compile_definitions(example_struct_to_json PRIVATE
    WWJSON_USE_SIMPLE_FLOAT_FORMAT=1
)

# Link with wwjson library
target_link_libraries(example_struct_to_json PRIVATE wwjson)

# Example 2: Estimate JSON size
add_executable(example_estimate_size
    estimate_size.cpp
)

# Add compile definitions for examples
target_compile_definitions(example_estimate_size PRIVATE
    WWJSON_USE_SIMPLE_FLOAT_FORMAT=1
)

# Link with wwjson library
target_link_libraries(example_estimate_size PRIVATE wwjson)

# Example 3: Hexadecimal JSON representation
add_executable(example_hex_json
    hex_json.cpp
)

# Add compile definitions for examples
target_compile_definitions(example_hex_json PRIVATE
    WWJSON_USE_SIMPLE_FLOAT_FORMAT=1
)

# Link with wwjson library
target_link_libraries(example_hex_json PRIVATE wwjson)

# Custom target to build all examples
add_custom_target(examples
    COMMAND ${CMAKE_COMMAND} -E echo "All example programs built"
    DEPENDS example_struct_to_json example_estimate_size example_hex_json
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building all example programs"
)

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
    DEPENDS examples
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all example programs"
)
