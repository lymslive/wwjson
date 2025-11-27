# CMakeLists.txt for unit tests
cmake_minimum_required(VERSION 3.15)

# Try to find couttast in local installation first (HOME directory)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{HOME})

# Find couttast dependency
find_package(couttast QUIET)

if(NOT couttast_FOUND)
    message(STATUS "couttast not found locally, fetching from GitHub...")
    
    # FetchContent to download couttast from GitHub
    include(FetchContent)
    
    # Try to find git repository
    find_program(GIT_EXECUTABLE git)
    if(NOT GIT_EXECUTABLE)
        message(FATAL_ERROR "Git not found. Cannot fetch couttast dependency.")
    endif()
    
    # Configure FetchContent for couttast
    FetchContent_Declare(
        couttast
        GIT_REPOSITORY https://github.com/lymslive/couttast.git
        GIT_TAG main  # or specific tag/commit
    )
    
    # Try to populate - if fails, provide helpful error message
    FetchContent_GetProperties(couttast)
    if(NOT couttast_POPULATED)
        message(STATUS "Attempting to download couttast from GitHub...")
        FetchContent_Populate(couttast)
    endif()
    
    # Add couttast as subdirectory if available
    if(EXISTS "${couttast_SOURCE_DIR}/CMakeLists.txt")
        add_subdirectory(${couttast_SOURCE_DIR} couttast-build)
    endif()
endif()

# Create test executable
add_executable(utwwjson
    # functional test files
    t_basic.cpp
    t_scope.cpp
    t_custom.cpp
    t_escape.cpp
    t_advance.cpp

    # just experiment/research test
    t_experiment.cpp

    # not test file, but used by other test file
    custom_string.cpp
)

# Link with wwjson library
target_link_libraries(utwwjson PRIVATE wwjson)

# Link with couttast - try different approaches
if(TARGET couttast::couttast)
    target_link_libraries(utwwjson PRIVATE couttast::couttast)
elseif(TARGET couttast)
    target_link_libraries(utwwjson PRIVATE couttast)
else()
    # Manual linking using standard paths
    find_path(COUTTAST_INCLUDE_DIR 
        NAMES couttast/tinytast.hpp 
        PATHS $ENV{HOME}/include /usr/local/include /usr/include
        PATH_SUFFIXES include
        NO_DEFAULT_PATH
    )
    find_library(COUTTAST_LIBRARY 
        NAMES couttast
        PATHS $ENV{HOME}/lib /usr/local/lib /usr/lib
        PATH_SUFFIXES lib
        NO_DEFAULT_PATH
    )
    
    if(COUTTAST_INCLUDE_DIR AND COUTTAST_LIBRARY)
        target_include_directories(utwwjson PRIVATE ${COUTTAST_INCLUDE_DIR})
        target_link_libraries(utwwjson PRIVATE ${COUTTAST_LIBRARY})
        message(STATUS "Found couttast: ${COUTTAST_LIBRARY}")
    else()
        message(FATAL_ERROR "Cannot find couttast library in local installation")
    endif()
endif()

# Add test to CTest if available
enable_testing()
add_test(NAME wwjson_unit_tests COMMAND utwwjson)

# Custom target to list tests (as mentioned in existing test.md command)
add_custom_target(list_tests
    COMMAND utwwjson --List
    DEPENDS utwwjson
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running wwjson unit tests with --List option"
)
