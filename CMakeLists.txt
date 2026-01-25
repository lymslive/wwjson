cmake_minimum_required(VERSION 3.15)
project(wwjson VERSION 1.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only library configuration
add_library(${PROJECT_NAME} INTERFACE)
add_library(wwjson::wwjson ALIAS wwjson)

# Target include directories
# Provide a build-time include layout that matches the installed layout
# by creating `${CMAKE_CURRENT_BINARY_DIR}/include/wwjson` as a symlink to
# the source `include` directory. This ensures `#include <wwjson/wwjson.hpp>`
# works both for in-tree builds (including FetchContent/subprojects) and
# for installed package usage.
set(_WWJSON_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${_WWJSON_BUILD_INCLUDE_DIR})
if(NOT EXISTS ${_WWJSON_BUILD_INCLUDE_DIR}/wwjson)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${_WWJSON_BUILD_INCLUDE_DIR}/wwjson
        RESULT_VARIABLE _wwjson_symlink_result
        OUTPUT_QUIET
        ERROR_QUIET
    )
endif()

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${_WWJSON_BUILD_INCLUDE_DIR}>
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

# Options to contorl build target:
# ---------------------------------------------------------------------- #

# Option to build only library (without tests)
option(WWJSON_LIB_ONLY "Build only library without tests" OFF)

# Option to use rapidjson's internal dtoa for floating-point serialization
option(WWJSON_USE_RAPIDJSON_DTOA "Use rapidjson internal dtoa for float serialization" OFF)

# Option to use fmt library for floating-point serialization
option(WWJSON_USE_FMTLIB_DTOA "Use fmt library for float serialization" OFF)

# Option to use yyjson library for floating-point serialization
option(WWJSON_USE_YYJSON_DTOA "Use yyjson library for float serialization (yyjson_dtoa)" OFF)

# Option to auto-detect and use external library for floating-point serialization
option(WWJSON_USE_EXTERNAL_DTOA "Auto-detect external dtoa library (rapidjson, fmt, or yyjson)" OFF)

# Option to build unit tests
option(BUILD_UNIT_TESTS "Build unit tests" ON)

# Option to build performance tests
option(BUILD_PERF_TESTS "Build performance tests" OFF)

# Option to build examples
option(BUILD_EXAMPLES "Build example programs" ON)

# ============================================================================
# External DTOA library support
# These libraries are used for high-performance floating-point serialization
# ============================================================================

# Track if we've already configured an external DTOA library
set(HAS_USED_EXTERNAL_DTOA FALSE)

if(WWJSON_USE_YYJSON_DTOA AND NOT HAS_USED_EXTERNAL_DTOA)
    find_package(yyjson QUIET)
    if(NOT yyjson_FOUND)
        message(STATUS "yyjson not found locally, fetching from GitHub...")
        FetchContent_Declare(
            yyjson
            GIT_REPOSITORY https://github.com/ibireme/yyjson.git
            GIT_TAG master
            )
        FetchContent_MakeAvailable(yyjson)
    endif()

    if(TARGET yyjson::yyjson)
        target_link_libraries(${PROJECT_NAME} INTERFACE yyjson::yyjson)
        target_compile_definitions(${PROJECT_NAME} INTERFACE WWJSON_USE_YYJSON_DTOA)
        set(HAS_USED_EXTERNAL_DTOA TRUE)

        get_target_property(YYJSON_INCLUDE_DIR yyjson::yyjson INTERFACE_INCLUDE_DIRECTORIES)
        message(STATUS "Using yyjson from: ${YYJSON_INCLUDE_DIR}")
        message(STATUS "WWJSON_USE_YYJSON_DTOA enabled")
    endif()
endif()

if(WWJSON_USE_FMTLIB_DTOA AND NOT HAS_USED_EXTERNAL_DTOA)
    find_package(fmt QUIET)
    if(NOT fmt_FOUND)
        message(STATUS "fmt not found, fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(fmt)
    endif()

    if(TARGET fmt::fmt)
        get_target_property(FMT_INCLUDE_DIR fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
        target_link_libraries(${PROJECT_NAME} INTERFACE fmt::fmt)
	target_compile_definitions(${PROJECT_NAME} INTERFACE WWJSON_USE_FMTLIB_DTOA)
	set(HAS_USED_EXTERNAL_DTOA TRUE)

	message(STATUS "Using fmt from: ${FMT_INCLUDE_DIR}")
	message(STATUS "WWJSON_USE_FMTLIB_DTOA enabled")
    endif()
endif()

if(WWJSON_USE_RAPIDJSON_DTOA AND NOT HAS_USED_EXTERNAL_DTOA)
    # Use find_path to locate rapidjson (header-only, no CMake target needed)
    find_path(RAPIDJSON_INCLUDE_DIR NAMES rapidjson/rapidjson.h PATHS /usr/local/include NO_DEFAULT_PATH)
    if(NOT RAPIDJSON_INCLUDE_DIR)
        message(STATUS "rapidjson not found, fetching from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            rapidjson
            GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(rapidjson)
        get_target_property(RAPIDJSON_INCLUDE_DIR rapidjson INTERFACE_INCLUDE_DIRECTORIES)
    endif()

    # Add include directory and compile definition to wwjson target
    target_include_directories(${PROJECT_NAME} INTERFACE ${RAPIDJSON_INCLUDE_DIR})
    target_compile_definitions(${PROJECT_NAME} INTERFACE WWJSON_USE_RAPIDJSON_DTOA)
    set(HAS_USED_EXTERNAL_DTOA TRUE)

    message(STATUS "Using rapidjson from: ${RAPIDJSON_INCLUDE_DIR}")
    message(STATUS "WWJSON_USE_RAPIDJSON_DTOA enabled")
endif()

# Auto-detect external DTOA library if WWJSON_USE_EXTERNAL_DTOA is ON
# and no specific library has been configured yet
if(WWJSON_USE_EXTERNAL_DTOA AND NOT HAS_USED_EXTERNAL_DTOA)
    message(STATUS "Auto-detecting external DTOA library...")

    # Try yyjson first (local installation only, no download)
    if(NOT HAS_USED_EXTERNAL_DTOA)
        find_package(yyjson QUIET)
        if(yyjson_FOUND AND TARGET yyjson::yyjson)
            target_link_libraries(${PROJECT_NAME} INTERFACE yyjson::yyjson)
            target_compile_definitions(${PROJECT_NAME} INTERFACE WWJSON_USE_YYJSON_DTOA)
            set(HAS_USED_EXTERNAL_DTOA TRUE)
            get_target_property(YYJSON_INCLUDE_DIR yyjson::yyjson INTERFACE_INCLUDE_DIRECTORIES)
            message(STATUS "Auto-detected yyjson from: ${YYJSON_INCLUDE_DIR}")
        endif()
    endif()

    # If yyjson not found, try fmt (local installation only, no download)
    if(NOT HAS_USED_EXTERNAL_DTOA)
        find_package(fmt QUIET)
        if(fmt_FOUND AND TARGET fmt::fmt)
            get_target_property(FMT_INCLUDE_DIR fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
            target_link_libraries(${PROJECT_NAME} INTERFACE fmt::fmt)
            target_compile_definitions(${PROJECT_NAME} INTERFACE WWJSON_USE_FMTLIB_DTOA)
            set(HAS_USED_EXTERNAL_DTOA TRUE)
            message(STATUS "Auto-detected fmt from: ${FMT_INCLUDE_DIR}")
        endif()
    endif()

    # rapidjson is header-only, so we use find_path instead of find_package
    if(NOT HAS_USED_EXTERNAL_DTOA)
        find_path(RAPIDJSON_INCLUDE_DIR NAMES rapidjson/rapidjson.h PATHS /usr/local/include NO_DEFAULT_PATH)
        if(RAPIDJSON_INCLUDE_DIR)
            target_include_directories(${PROJECT_NAME} INTERFACE ${RAPIDJSON_INCLUDE_DIR})
            target_compile_definitions(${PROJECT_NAME} INTERFACE WWJSON_USE_RAPIDJSON_DTOA)
            set(HAS_USED_EXTERNAL_DTOA TRUE)
            message(STATUS "Auto-detected rapidjson from: ${RAPIDJSON_INCLUDE_DIR}")
        endif()
    endif()

    if(NOT HAS_USED_EXTERNAL_DTOA)
        message(STATUS "No external DTOA library found, using internal implementation")
    endif()
endif()

# For development targets:
# ---------------------------------------------------------------------- #

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
        set(COUTTAST_LIB_ONLY ON CACHE BOOL "Build only the couttast library" FORCE)
        
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
        set(XYJSON_LIB_ONLY ON CACHE BOOL "Build only the xyjson library" FORCE)
        
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
else()
    message(STATUS "WWJSON_LIB_ONLY is ON: skipping targets 'utest', 'perf', and 'example'.")
endif()
