# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

cmake_minimum_required(VERSION 3.12)

project(Kuku VERSION 1.1.1 LANGUAGES CXX C)

if(${MSVC})
    message(FATAL_ERROR "Please build using the attached Visual Studio solution/project files.")
endif()

# Build in Release mode by default; otherwise use selected option
set(KUKU_DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${KUKU_DEFAULT_BUILD_TYPE} CACHE
        STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Release" "Debug" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Build type (CMAKE_BUILD_TYPE): ${CMAKE_BUILD_TYPE}")

# In Debug mode enable also KUKU_DEBUG by default
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(KUKU_DEBUG_DEFAULT ON)
else()
    set(KUKU_DEBUG_DEFAULT OFF)
endif()

# Are we using KUKU_DEBUG?
set(KUKU_DEBUG ${KUKU_DEBUG_DEFAULT})
message(STATUS "Kuku debug mode: ${KUKU_DEBUG}")

# Should we build a shared library?
set(KUKU_DEFAULT_LIB_BUILD_TYPE "Static_PIC")
if(NOT KUKU_LIB_BUILD_TYPE)
    set(KUKU_LIB_BUILD_TYPE ${KUKU_DEFAULT_LIB_BUILD_TYPE} CACHE
        STRING "Library build type" FORCE)
    set_property(CACHE KUKU_LIB_BUILD_TYPE PROPERTY STRINGS
        "Static" "Static_PIC" "Shared")
endif()
message(STATUS "Library build type (KUKU_LIB_BUILD_TYPE): ${KUKU_LIB_BUILD_TYPE}")

# Optionally enable CMAKE_POSITION_INDEPENDENT_CODE
if(SEAL_LIB_BUILD_TYPE STREQUAL "Static")
    set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
else()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Required files and directories
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Kuku_SOURCE_DIR}/../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Kuku_SOURCE_DIR}/../lib)
set(CMAKE_LIBRARY_RUNTIME_DIRECTORY ${Kuku_SOURCE_DIR}/../lib)
set(KUKU_TARGETS_FILENAME ${Kuku_SOURCE_DIR}/cmake/KukuTargets.cmake)
set(KUKU_CONFIG_IN_FILENAME ${Kuku_SOURCE_DIR}/cmake/KukuConfig.cmake.in)
set(KUKU_CONFIG_FILENAME ${Kuku_SOURCE_DIR}/cmake/KukuConfig.cmake)
set(KUKU_CONFIG_VERSION_FILENAME ${Kuku_SOURCE_DIR}/cmake/KukuConfigVersion.cmake)
set(KUKU_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Kuku-${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
set(KUKU_INCLUDES_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/Kuku-${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})

# Create library but add no source files yet
if(KUKU_LIB_BUILD_TYPE STREQUAL "Shared")
    add_library(kuku SHARED "")

    # Set SOVERSION for shared library
    set_target_properties(kuku PROPERTIES
        SOVERSION ${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
else()
    add_library(kuku STATIC "")

    # For a static library indicate version in the filename
    set_target_properties(kuku PROPERTIES OUTPUT_NAME kuku-${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
endif()

# Set VERSION for all library build types
set_target_properties(kuku PROPERTIES VERSION ${Kuku_VERSION})

# Add source files to library and header files to install
add_subdirectory(kuku)

# Add local include directories for build
target_include_directories(kuku PUBLIC
    $<BUILD_INTERFACE:${Kuku_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${KUKU_INCLUDES_INSTALL_DIR}>)

# Require C++14
target_compile_features(kuku PUBLIC cxx_std_14)

# Associate Kuku to export kuku_export
install(TARGETS kuku EXPORT KukuTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Create the CMake config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${KUKU_CONFIG_IN_FILENAME} ${KUKU_CONFIG_FILENAME}
    INSTALL_DESTINATION ${KUKU_CONFIG_INSTALL_DIR})

# Install the export
install(
    EXPORT KukuTargets
    NAMESPACE Kuku::
    DESTINATION ${KUKU_CONFIG_INSTALL_DIR})

# Version file; we require exact version match for downstream
write_basic_package_version_file(
    ${KUKU_CONFIG_VERSION_FILENAME}
    VERSION ${Kuku_VERSION}
    COMPATIBILITY SameMinorVersion)

# Install other files
install(
    FILES
        ${KUKU_CONFIG_FILENAME}
        ${KUKU_CONFIG_VERSION_FILENAME}
    DESTINATION ${KUKU_CONFIG_INSTALL_DIR})

# We export KukuTargets from the build tree so it can be used by other projects
# without requiring an install.
export(
    EXPORT KukuTargets
    NAMESPACE Kuku::
    FILE ${KUKU_TARGETS_FILENAME})
