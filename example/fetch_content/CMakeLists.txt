# CMakeLists.txt for FetchContent usage example
# This demonstrates how to integrate wwjson without system-wide installation
cmake_minimum_required(VERSION 3.15)
project(wwjson_fetch_content_example LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use FetchContent to download wwjson from GitHub
include(FetchContent)

# Configure FetchContent for wwjson
FetchContent_Declare(
    wwjson
    GIT_REPOSITORY https://github.com/lymslive/wwjson.git
    GIT_TAG dev  # or specific tag/commit
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

set(WWJSON_LIB_ONLY ON CACHE BOOL "Build only the header-only library" FORCE)

# Make wwjson available
FetchContent_MakeAvailable(wwjson)

# Determine the correct target name depending on whether wwjson was
# added as an exported/imported target (`wwjson::wwjson`) or as a
# plain target (`wwjson`) when used as a subproject.
if(TARGET wwjson::wwjson)
    set(WWJSON_TARGET wwjson::wwjson)
elseif(TARGET wwjson)
    set(WWJSON_TARGET wwjson)
else()
    message(FATAL_ERROR "wwjson target not found; expected wwjson::wwjson or wwjson")
endif()

# Example 1: struct_to_json
add_executable(example_struct_to_json
    ${CMAKE_CURRENT_SOURCE_DIR}/../struct_to_json.cpp
)
target_link_libraries(example_struct_to_json PRIVATE ${WWJSON_TARGET})

# Example 2: estimate_size
add_executable(example_estimate_size
    ${CMAKE_CURRENT_SOURCE_DIR}/../estimate_size.cpp
)
target_link_libraries(example_estimate_size PRIVATE ${WWJSON_TARGET})

# Example 3: hex_json
add_executable(example_hex_json
    ${CMAKE_CURRENT_SOURCE_DIR}/../hex_json.cpp
)
target_link_libraries(example_hex_json PRIVATE ${WWJSON_TARGET})

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
