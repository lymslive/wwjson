# CMakeLists.txt for unit tests
cmake_minimum_required(VERSION 3.15)

# Find xyjson dependency (only for unit tests)
find_package(xyjson QUIET)
if(NOT xyjson_FOUND)
    message(STATUS "xyjson not found locally, fetching from GitHub...")
    
    # Configure FetchContent for xyjson
    FetchContent_Declare(
        xyjson
        GIT_REPOSITORY https://github.com/lymslive/xyjson.git
        GIT_TAG main  # or specific tag/commit
        OPTIONS "-DXYJSON_LIB_ONLY=ON"
    )
    FetchContent_MakeAvailable(xyjson)
endif()

# Get include directory for xyjson
if(TARGET xyjson::xyjson)
    get_target_property(XYJSON_INCLUDE_DIR xyjson::xyjson INTERFACE_INCLUDE_DIRECTORIES)
endif()

# Report what we found
message(STATUS "Using xyjson from: ${XYJSON_INCLUDE_DIR}")

# Create test executable
add_executable(utwwjson
    # functional test files
    t_basic.cpp
    t_number.cpp
    t_scope.cpp
    t_custom.cpp
    t_escape.cpp
    t_advance.cpp
    t_operator.cpp

    # just experiment/research test
    t_experiment.cpp

    # not test file, but used by other test file
    custom_string.cpp
    test_util.cpp
)

# Add compile definition for simple float format in development
target_compile_definitions(utwwjson PRIVATE WWJSON_USE_SIMPLE_FLOAT_FORMAT=1)

# Link with wwjson library
target_link_libraries(utwwjson PRIVATE wwjson)

# Link with modern CMake targets if available
if(TARGET couttast::couttast)
    target_link_libraries(utwwjson PRIVATE couttast::couttast)
elseif(TARGET couttast)
    target_link_libraries(utwwjson PRIVATE couttast)
endif()

if(TARGET yyjson::yyjson)
    target_link_libraries(utwwjson PRIVATE yyjson::yyjson)
endif()

if(TARGET xyjson::xyjson)
    target_link_libraries(utwwjson PRIVATE xyjson::xyjson)
endif()

# Add test to CTest if available
enable_testing()
add_test(NAME wwjson_unit_tests COMMAND utwwjson)

# Custom target to list tests
add_custom_target(list_tests
    COMMAND utwwjson --List
    DEPENDS utwwjson
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running wwjson unit tests with --List option"
)
