cmake_minimum_required(VERSION 3.15)
project(wwjson VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only library configuration
add_library(${PROJECT_NAME} INTERFACE)

# Target include directories
target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Export library
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Install all header files to wwjson/ subdirectory
file(GLOB WWJSON_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
install(FILES ${WWJSON_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/wwjson
)

# Install CMake configuration files
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Generate config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# For development targets:
# ---------------------------------------------------------------------- #

# Option to build only library (without tests)
option(WWJSON_LIB_ONLY "Build only library without tests" OFF)

# Option to build unit tests
option(BUILD_UNIT_TESTS "Build unit tests" ON)

# Option to build performance tests
option(BUILD_PERF_TESTS "Build performance tests" OFF)

# Option to build examples
option(BUILD_EXAMPLES "Build example programs" ON)

# Dependency management for tests and performance benchmarks
if(NOT WWJSON_LIB_ONLY)
    # Try to find couttast in local installation first (HOME directory)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{HOME})

    # FetchContent to download dependency from GitHub if not found locally.
    include(FetchContent)

    # Find couttast dependency
    find_package(couttast QUIET)
    if(NOT couttast_FOUND)
        message(STATUS "couttast not found locally, fetching from GitHub...")
        
        # Configure FetchContent for couttast
        FetchContent_Declare(
            couttast
            GIT_REPOSITORY https://github.com/lymslive/couttast.git
            GIT_TAG main  # or specific tag/commit
        )
        FetchContent_MakeAvailable(couttast)
    endif()

    # Find yyjson dependency
    find_package(yyjson QUIET)
    if(NOT yyjson_FOUND)
        message(STATUS "yyjson not found locally, fetching from GitHub...")
        
        # Configure FetchContent for yyjson
        FetchContent_Declare(
            yyjson
            GIT_REPOSITORY https://github.com/ibireme/yyjson.git
            GIT_TAG master  # or specific tag/commit
        )
        FetchContent_MakeAvailable(yyjson)
    endif()

    # Find xyjson dependency (for both unit tests and performance tests)
    find_package(xyjson QUIET)
    if(NOT xyjson_FOUND)
        message(STATUS "xyjson not found locally, fetching from GitHub...")
        set(XYJSON_LIB_ONLY ON CACHE BOOL "Build only the header-only library" FORCE)
        
        # Configure FetchContent for xyjson
        FetchContent_Declare(
            xyjson
            GIT_REPOSITORY https://github.com/lymslive/xyjson.git
            GIT_TAG main  # or specific tag/commit
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(xyjson)
        message(STATUS "XYJSON_LIB_ONLY = ${XYJSON_LIB_ONLY}")
    endif()

    # Get include directories from found packages
    if(TARGET couttast)
        get_target_property(COUTTAST_INCLUDE_DIR couttast INTERFACE_INCLUDE_DIRECTORIES)
    endif()

    if(TARGET yyjson::yyjson)
        get_target_property(YYJSON_INCLUDE_DIR yyjson::yyjson INTERFACE_INCLUDE_DIRECTORIES)
    endif()

    # Get include directory for xyjson
    if(TARGET xyjson::xyjson)
        get_target_property(XYJSON_INCLUDE_DIR xyjson::xyjson INTERFACE_INCLUDE_DIRECTORIES)
    elseif(TARGET xyjson)
        get_target_property(XYJSON_INCLUDE_DIR xyjson INTERFACE_INCLUDE_DIRECTORIES)
    endif()

    # Report what we found
    message(STATUS "Using couttast from: ${COUTTAST_INCLUDE_DIR}")
    message(STATUS "Using yyjson from: ${YYJSON_INCLUDE_DIR}")
    message(STATUS "Using xyjson from: ${XYJSON_INCLUDE_DIR}")
endif()

# Add subdirectories for tests and performance benchmarks
if(NOT WWJSON_LIB_ONLY)
    # Add unit tests subdirectory if enabled
    if(BUILD_UNIT_TESTS)
        add_subdirectory(utest)
    endif()
    
    # Add performance tests subdirectory if enabled
    if(BUILD_PERF_TESTS)
        add_subdirectory(perf)
    endif()

    # Add examples subdirectory if enabled
    if(BUILD_EXAMPLES)
        add_subdirectory(example)
    endif()
endif()
