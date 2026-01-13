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
#       GIT_REPOSITORY git@github.com:lymslive/xyjson.git
        GIT_TAG main  # or specific tag/commit
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OPTIONS "-DXYJSON_LIB_ONLY=ON" "-DUSE_FETCHCONTENT=OFF"
    )
    FetchContent_MakeAvailable(xyjson)
endif()

# Get include directory for xyjson
if(TARGET xyjson::xyjson)
    get_target_property(XYJSON_INCLUDE_DIR xyjson::xyjson INTERFACE_INCLUDE_DIRECTORIES)
endif()

# Report what we found
message(STATUS "Using xyjson from: ${XYJSON_INCLUDE_DIR}")

# Create test executable for unit tests
add_executable(utwwjson
    # functional test files
    t_basic.cpp
    t_number.cpp
    t_itoa.cpp
    t_scope.cpp
    t_custom.cpp
    t_escape.cpp
    t_advance.cpp
    t_operator.cpp
    t_bufferview.cpp
    t_jstring.cpp
    t_jbuilder.cpp

    # just experiment/research test
    t_experiment.cpp

    # not test file, but used by other test file
    custom_string.cpp
    test_util.cpp
)

# Add compile definitions for unit tests
target_compile_definitions(utwwjson PRIVATE
    WWJSON_USE_SIMPLE_FLOAT_FORMAT=1
    JSTRING_MAX_EXP_ALLOC_SIZE=1024  # Smaller for testing memory growth
)

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
elseif(TARGET yyjson)
    target_link_libraries(utwwjson PRIVATE yyjson)
endif()

if(TARGET xyjson::xyjson)
    target_link_libraries(utwwjson PRIVATE xyjson::xyjson)
elseif(TARGET xyjson)
    target_link_libraries(utwwjson PRIVATE xyjson)
endif()

# Add test to CTest if available
enable_testing()
add_test(NAME wwjson_unit_tests COMMAND utwwjson)

# Create separate executable for documentation tests
add_executable(utdocs
    # test sample from docs
    t_usage.cpp
)

# Add compile definitions for docs tests
target_compile_definitions(utdocs PRIVATE
    WWJSON_USE_SIMPLE_FLOAT_FORMAT=1
)

# Link with wwjson library
target_link_libraries(utdocs PRIVATE wwjson)

# Link with modern CMake targets if available
if(TARGET couttast::couttast)
    target_link_libraries(utdocs PRIVATE couttast::couttast)
elseif(TARGET couttast)
    target_link_libraries(utdocs PRIVATE couttast)
endif()

# Add test to CTest if available
enable_testing()
add_test(NAME wwjson_docs_tests COMMAND utdocs --cout=silent)

# Custom target to list tests
add_custom_target(list_tests
    COMMAND utwwjson --List
    DEPENDS utwwjson
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running wwjson unit tests with --List option"
)
