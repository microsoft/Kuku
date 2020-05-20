# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

cmake_minimum_required(VERSION 3.12)

###################################################
# Project Kuku includes the following components: #
#   1. Kuku C++ library                           #
#   2. Kuku C export library                      #
#   3. Kuku C++ examples                          #
#   4. Kuku C++ tests                             #
###################################################

project(Kuku VERSION 2.2.0 LANGUAGES CXX C)

if(${MSVC})
    message(FATAL_ERROR "Please build using the attached Visual Studio solution/project files.")
endif()

########################
# Global configuration #
########################

# [Option] CMAKE_BUILD_TYPE
# Build in Release mode by default; otherwise use selected option
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Release" "Debug" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Build type (CMAKE_BUILD_TYPE): ${CMAKE_BUILD_TYPE}")

# [Flag] KUKU_DEBUG
# In Debug mode, enable KUKU_DEBUG
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(KUKU_DEBUG ON)
else()
    set(KUKU_DEBUG OFF)
endif()
message(STATUS "Kuku debug mode: ${KUKU_DEBUG}")

# Required files and directories
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Kuku_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Kuku_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_RUNTIME_DIRECTORY ${Kuku_SOURCE_DIR}/bin)
set(KUKU_TARGETS_FILENAME ${Kuku_SOURCE_DIR}/cmake/KukuTargets.cmake)
set(KUKU_CONFIG_IN_FILENAME ${Kuku_SOURCE_DIR}/cmake/KukuConfig.cmake.in)
set(KUKU_CONFIG_FILENAME ${Kuku_SOURCE_DIR}/cmake/KukuConfig.cmake)
set(KUKU_CONFIG_VERSION_FILENAME ${Kuku_SOURCE_DIR}/cmake/KukuConfigVersion.cmake)
set(KUKU_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Kuku-${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
set(KUKU_INCLUDES_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/Kuku-${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
set(KUKU_INCLUDES_BUILD_DIR ${Kuku_SOURCE_DIR}/src)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Kuku_SOURCE_DIR}/bin)

include(CMakePushCheckState)
include(CMakeDependentOption)
include(CheckIncludeFiles)
include(CheckCXXSourceRuns)
include(CheckTypeSize)

##################################
# Macros for configuring targets #
##################################

# Set the C++ language version
macro(kuku_set_language target)
    target_compile_features(${target} PUBLIC cxx_std_14)
endmacro()

# Set the VERSION property
macro(kuku_set_version target)
    set_target_properties(${target} PROPERTIES VERSION ${Kuku_VERSION})
endmacro()

# Set the library filename to reflect version
macro(kuku_set_version_filename target)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME ${target}-${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
endmacro()

# Set the SOVERSION property
macro(kuku_set_soversion target)
    set_target_properties(${target} PROPERTIES
        SOVERSION ${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
endmacro()

# Set include directories for build and install interfaces
macro(kuku_set_include_directories target)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${KUKU_INCLUDES_BUILD_DIR}>
        $<INSTALL_INTERFACE:${KUKU_INCLUDES_INSTALL_DIR}>)
endmacro()

# Include target to given export
macro(kuku_install_target target export)
    install(TARGETS ${target} EXPORT ${export}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()

#########################
# External dependencies #
#########################

set(THIRDPARTY_DIR ${Kuku_SOURCE_DIR}/thirdparty)

# Google Test
# This follows the example in
# https://github.com/google/googletest/blob/release-1.10.0/googletest/README.md.

# Download and configure
if(KUKU_BUILD_TESTS AND NOT MSVC)
    message(STATUS "Setting up Google Test ...")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        OUTPUT_QUIET
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${THIRDPARTY_DIR}/googletest)
    if(result)
        message(WARNING "Failed to download Google Test (${result}); disabling `KUKU_BUILD_TESTS`")
    endif()
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    mark_as_advanced(BUILD_GMOCK)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    mark_as_advanced(INSTALL_GTEST)
endif()

# Build
if(KUKU_BUILD_TESTS AND NOT MSVC)
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
        OUTPUT_QUIET
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${THIRDPARTY_DIR}/googletest)
    if(result)
        message(WARNING "Failed to build Google Test (${result}); disabling `KUKU_BUILD_TESTS`")
    endif()
endif()

# Set up the targets
if(KUKU_BUILD_TESTS AND NOT MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(
        ${THIRDPARTY_DIR}/googletest/src
        ${THIRDPARTY_DIR}/googletest/build
        EXCLUDE_FROM_ALL)
endif()

####################
# Kuku C++ library #
####################

# Should we build also the shared library?
set(BUILD_SHARED_LIBS_STR "Build shared library")
option(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_STR} OFF)
if(MSVC AND BUILD_SHARED_LIBS)
    message(WARNING "This build system only supports a static build; disabling `BUILD_SHARED_LIBS`")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL ${BUILD_SHARED_LIBS_STR} FORCE)
endif()

# Always build position-independent-code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Create an object library to compile sources only once
add_library(kuku_obj OBJECT)

# Add source files to library and header files to install
add_subdirectory(src/kuku)

# Set C++ language version and include directories for the object library
kuku_set_language(kuku_obj)
kuku_set_include_directories(kuku_obj)

# Always build the static library
add_library(kuku STATIC $<TARGET_OBJECTS:kuku_obj>)
kuku_set_version(kuku)
kuku_set_version_filename(kuku)
kuku_set_language(kuku)
kuku_set_include_directories(kuku)

kuku_install_target(kuku KukuTargets)

# Conditionally build the shared library
if(BUILD_SHARED_LIBS)
    add_library(kuku_shared SHARED $<TARGET_OBJECTS:kuku_obj>)
    set_target_properties(kuku_shared PROPERTIES OUTPUT_NAME kuku)
    kuku_set_version(kuku_shared)
    kuku_set_soversion(kuku_shared)
    kuku_set_language(kuku_shared)
    kuku_set_include_directories(kuku_shared)
    kuku_install_target(kuku_shared KukuTargets)
endif()

#########################
# Kuku C export library #
#########################

# Check that size_t is 8 bytes
include(CheckTypeSize)
check_type_size("size_t" SIZET LANGUAGE C)

set(KUKU_BUILD_KUKU_C_OPTION_STR "Build C export library for Kuku")
cmake_dependent_option(KUKU_BUILD_KUKU_C ${KUKU_BUILD_KUKU_C_OPTION_STR} OFF "${SIZET} EQUAL 8" OFF)

# Create shared Kuku_C library but add no source files yet
if(KUKU_BUILD_KUKU_C)
    add_library(kukuc SHARED)

    # Add source files to library and header files to install
    add_subdirectory(src/kuku/c)
    kuku_set_version(kukuc)
    kuku_set_soversion(kukuc)
    kuku_set_language(kukuc)
    kuku_set_include_directories(kukuc)
    target_link_libraries(kukuc PUBLIC kuku)
    kuku_install_target(kukuc KukuTargets)
endif()

#################################
# Installation and CMake config #
#################################

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

# Install config and version files
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

#####################
# Kuku C++ examples #
#####################

set(KUKU_BUILD_EXAMPLES_OPTION_STR "Build C++ examples for Kuku")
option(KUKU_BUILD_EXAMPLES ${KUKU_BUILD_EXAMPLES_OPTION_STR} OFF)

if(KUKU_BUILD_EXAMPLES)
    add_executable(kukuexamples)
    add_subdirectory(examples)
    target_link_libraries(kukuexamples PUBLIC kuku)
endif()

##################
# Kuku C++ tests #
##################

set(KUKU_BUILD_TESTS_OPTION_STR "Build C++ tests for Kuku")
option(KUKU_BUILD_TESTS ${KUKU_BUILD_TESTS_OPTION_STR} OFF)

if(KUKU_BUILD_TESTS)
    add_executable(kukutest)
    add_subdirectory(tests/kuku)
    target_link_libraries(kukutest PRIVATE kuku gtest)
endif()
