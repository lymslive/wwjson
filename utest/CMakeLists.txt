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

# Find yyjson dependency
find_package(yyjson QUIET)

if(NOT yyjson_FOUND)
    message(STATUS "yyjson not found locally, fetching from GitHub...")
    
    # FetchContent to download yyjson from GitHub
    include(FetchContent)
    
    # Configure FetchContent for yyjson
    FetchContent_Declare(
        yyjson
        GIT_REPOSITORY https://github.com/ibireme/yyjson.git
        GIT_TAG main  # or specific tag/commit
    )
    
    # Try to populate - if fails, provide helpful error message
    FetchContent_GetProperties(yyjson)
    if(NOT yyjson_POPULATED)
        message(STATUS "Attempting to download yyjson from GitHub...")
        FetchContent_Populate(yyjson)
    endif()
    
    # Add yyjson as subdirectory if available
    if(EXISTS "${yyjson_SOURCE_DIR}/CMakeLists.txt")
        add_subdirectory(${yyjson_SOURCE_DIR} yyjson-build)
    endif()
endif()

# Find xyjson dependency
find_package(xyjson QUIET)

if(NOT xyjson_FOUND)
    message(STATUS "xyjson not found locally, fetching from GitHub...")
    
    # FetchContent to download xyjson from GitHub
    include(FetchContent)
    
    # Configure FetchContent for xyjson
    FetchContent_Declare(
        xyjson
        GIT_REPOSITORY https://github.com/lymslive/xyjson.git
        GIT_TAG main  # or specific tag/commit
    )
    
    # Try to populate - if fails, provide helpful error message
    FetchContent_GetProperties(xyjson)
    if(NOT xyjson_POPULATED)
        message(STATUS "Attempting to download xyjson from GitHub...")
        FetchContent_Populate(xyjson)
    endif()
    
    # Add xyjson as subdirectory if available
    if(EXISTS "${xyjson_SOURCE_DIR}/CMakeLists.txt")
        add_subdirectory(${xyjson_SOURCE_DIR} xyjson-build)
    endif()
endif()

# Get include directories from found packages
if(couttast_FOUND AND TARGET couttast)
    get_target_property(COUTTAST_INCLUDE_DIR couttast INTERFACE_INCLUDE_DIRECTORIES)
endif()

if(yyjson_FOUND AND TARGET yyjson::yyjson)
    get_target_property(YYJSON_INCLUDE_DIR yyjson::yyjson INTERFACE_INCLUDE_DIRECTORIES)
endif()

if(xyjson_FOUND AND TARGET xyjson::xyjson)
    get_target_property(XYJSON_INCLUDE_DIR xyjson::xyjson INTERFACE_INCLUDE_DIRECTORIES)
elseif(xyjson_FOUND)
    # Fallback for header-only libraries
    set(XYJSON_INCLUDE_DIR "${xyjson_SOURCE_DIR}/include")
endif()

# Report what we found
message(STATUS "Using couttast from: ${COUTTAST_INCLUDE_DIR}")
message(STATUS "Using yyjson from: ${YYJSON_INCLUDE_DIR}")
message(STATUS "Using xyjson from: ${XYJSON_INCLUDE_DIR}")

# Create test executable
add_executable(utwwjson
    # functional test files
    t_basic.cpp
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

# Link with wwjson library
target_link_libraries(utwwjson PRIVATE wwjson)

# Add include directories for JSON validation dependencies
if(XYJSON_INCLUDE_DIR)
    target_include_directories(utwwjson PRIVATE ${XYJSON_INCLUDE_DIR})
endif()
if(YYJSON_INCLUDE_DIR)
    target_include_directories(utwwjson PRIVATE ${YYJSON_INCLUDE_DIR})
endif()
if(COUTTAST_INCLUDE_DIR)
    target_include_directories(utwwjson PRIVATE ${COUTTAST_INCLUDE_DIR})
endif()

# Link with modern CMake targets if available
if(TARGET couttast::couttast)
    target_link_libraries(utwwjson PRIVATE couttast::couttast)
elseif(TARGET couttast)
    target_link_libraries(utwwjson PRIVATE couttast)
endif()

if(TARGET yyjson::yyjson)
    target_link_libraries(utwwjson PRIVATE yyjson::yyjson)
elseif(YYJSON_LIBRARY)
    target_link_libraries(utwwjson PRIVATE ${YYJSON_LIBRARY})
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
