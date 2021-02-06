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

# [option] CMAKE_BUILD_TYPE (default: "Release")
# Build in one of the following modes: Release, Debug, MiniSizeRel, or RelWithDebInfo.
# Most generators recognize these and can set the compiler flags accordingly. We set
# the build type here before creating the project to prevent the CMake generator from
# overriding our default of "Release".
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Release" "Debug" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Build type (CMAKE_BUILD_TYPE): ${CMAKE_BUILD_TYPE}")

project(Kuku VERSION 2.1.0 LANGUAGES CXX C)

########################
# Global configuration #
########################

include(CMakePushCheckState)
include(CMakeDependentOption)
include(CheckIncludeFiles)
include(CheckCXXSourceRuns)
include(CheckTypeSize)
include(FetchContent)

mark_as_advanced(FETCHCONTENT_BASE_DIR)
mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_QUIET)

# Extra modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Always build position-independent-code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Make the install target depend on the all target (required by vcpkg)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY OFF)

# In Debug mode, enable KUKU_DEBUG
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(KUKU_DEBUG ON)
else()
    set(KUKU_DEBUG OFF)
endif()
message(STATUS "Kuku debug mode: ${KUKU_DEBUG}")

# Enable security-related compile options (MSVC only)
set(KUKU_SECURE_COMPILE_OPTIONS_OPTIONS_STR "Enable Control Flow Guard and Spectre mitigations (MSVC only)")
option(KUKU_SECURE_COMPILE_OPTIONS ${KUKU_SECURE_COMPILE_OPTIONS_OPTIONS_STR} OFF)
mark_as_advanced(KUKU_SECURE_COMPILE_OPTIONS)

# Required files and directories
include(GNUInstallDirs)

# Source Tree
set(KUKU_INCLUDES_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(KUKU_THIRDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty)
set(KUKU_CONFIG_IN_FILENAME ${CMAKE_CURRENT_LIST_DIR}/cmake/KukuConfig.cmake.in)
set(KUKU_CONFIG_H_IN_FILENAME ${KUKU_INCLUDES_DIR}/kuku/internal/config.h.in)

# Build tree
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(KUKU_TARGETS_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/KukuTargets.cmake)
set(KUKU_CONFIG_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/KukuConfig.cmake)
set(KUKU_CONFIG_VERSION_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/KukuConfigVersion.cmake)
set(KUKU_CONFIG_H_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/src/kuku/internal/config.h)

# Install
set(KUKU_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Kuku-${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
set(KUKU_INCLUDES_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/Kuku-${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})

##################
# Various macros #
##################

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

# Set include directories for build and install interfaces
macro(kuku_set_include_directories target)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${KUKU_INCLUDES_DIR}>
        $<INSTALL_INTERFACE:${KUKU_INCLUDES_INSTALL_DIR}>)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src/>
        $<INSTALL_INTERFACE:${KUKU_INCLUDES_INSTALL_DIR}>)
endmacro()

# Include target to given export
macro(kuku_install_target target export)
    install(TARGETS ${target} EXPORT ${export}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()

# Add secure compile options
macro(kuku_set_secure_compile_options target scope)
    if(MSVC)
        # Build debug symbols for static analysis tools
        target_link_options(${target} ${scope} /DEBUG)

        # Control Flow Guard / Spectre
        target_compile_options(${target} ${scope} /guard:cf)
        target_compile_options(${target} ${scope} /Qspectre)
        target_link_options(${target} ${scope} /guard:cf)
        target_link_options(${target} ${scope} /DYNAMICBASE)
    endif()
endmacro()

# Include a file to fetch thirdparty content
macro(kuku_fetch_thirdparty_content content_file)
    set(KUKU_FETCHCONTENT_BASE_DIR_OLD ${FETCHCONTENT_BASE_DIR})
    set(FETCHCONTENT_BASE_DIR ${KUKU_THIRDPARTY_DIR} CACHE STRING "" FORCE)
    include(${content_file})
    set(FETCHCONTENT_BASE_DIR ${KUKU_FETCHCONTENT_BASE_DIR_OLD} CACHE STRING "" FORCE)
    unset(KUKU_FETCHCONTENT_BASE_DIR_OLD)
endmacro()

####################
# Kuku C++ library #
####################

# Should we build also the shared library?
set(BUILD_SHARED_LIBS_STR "Build shared library")
option(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_STR} OFF)
if(WIN32 AND BUILD_SHARED_LIBS)
    message(FATAL_ERROR "On Windows only static build is supported; set `BUILD_SHARED_LIBS=OFF`")
endif()

# Add source files to library and header files to install
set(KUKU_SOURCE_FILES "")
add_subdirectory(src/kuku)

# Create the config file
configure_file(${KUKU_CONFIG_H_IN_FILENAME} ${KUKU_CONFIG_H_FILENAME})
install(
    FILES ${KUKU_CONFIG_H_FILENAME}
    DESTINATION ${KUKU_INCLUDES_INSTALL_DIR}/kuku/internal)

set(KUKU_LIBRARY_NAME "")
if(NOT BUILD_SHARED_LIBS)
    add_library(kuku STATIC ${KUKU_SOURCE_FILES})
    kuku_set_version_filename(kuku)
    kuku_set_language(kuku)
    kuku_set_include_directories(kuku)
    kuku_set_version(kuku)
    kuku_install_target(kuku KukuTargets)
    set(KUKU_LIBRARY_NAME "kuku")

    # Set secure compile options if KUKU_SECURE_COMPILE_OPTIONS is ON; only supported on MSVC
    if(KUKU_SECURE_COMPILE_OPTIONS)
        kuku_set_secure_compile_options(kuku PUBLIC)
    endif()

# Build only a shared library
else()
    add_library(kuku_shared SHARED ${KUKU_SOURCE_FILES})
    kuku_set_soversion(kuku_shared)
    set_target_properties(kuku_shared PROPERTIES OUTPUT_NAME kuku)
    kuku_set_language(kuku_shared)
    kuku_set_include_directories(kuku_shared)
    kuku_set_version(kuku_shared)
    kuku_install_target(kuku_shared KukuTargets)
    set(KUKU_LIBRARY_NAME "kuku_shared")
endif()

# Add standard alias targets for Kuku::kuku and Kuku::kuku_shared
if(TARGET kuku)
    add_library(Kuku::kuku ALIAS kuku)
endif()
if(TARGET kuku_shared)
    add_library(Kuku::kuku_shared ALIAS kuku_shared)
endif()

#########################
# Kuku C export library #
#########################

set(KUKU_BUILD_KUKU_C_OPTION_STR "Build C export library for Kuku")
option(KUKU_BUILD_KUKU_C ${KUKU_BUILD_KUKU_C_OPTION_STR} OFF)

# Create shared Kuku_C library but add no source files yet
if(KUKU_BUILD_KUKU_C)
    # Check that size_t is 8 bytes
    include(CheckTypeSize)
    check_type_size("size_t" SIZET LANGUAGE C)
    if(NOT ${SIZET} EQUAL 8)
        unset(SIZET CACHE)
        unset(HAVE_SIZET CACHE)
        message(FATAL_ERROR "Kuku_C requires 64-bit platform")
    endif()
    unset(SIZET CACHE)
    unset(HAVE_SIZET CACHE)

    if(BUILD_SHARED_LIBS)
        message(FATAL_ERROR "Kuku_C requires `BUILD_SHARED_LIBS=OFF`")
    endif()

    add_library(kukuc SHARED)

    # Add source files to library and header files to install
    add_subdirectory(src/kuku/c)
    kuku_set_version(kukuc)
    kuku_set_soversion(kukuc)
    kuku_set_language(kukuc)
    kuku_set_include_directories(kukuc)

    target_link_libraries(kukuc PUBLIC ${KUKU_LIBRARY_NAME})

    kuku_install_target(kukuc KukuTargets)
endif()

# Add standard alias target for Kuku::kukuc
if(TARGET kukuc)
    add_library(Kuku::kukuc ALIAS kukuc)
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
    target_link_libraries(kukuexamples PRIVATE ${KUKU_LIBRARY_NAME})
endif()

##################
# Kuku C++ tests #
##################

set(KUKU_BUILD_TESTS_OPTION_STR "Build C++ tests for Kuku")
option(KUKU_BUILD_TESTS ${KUKU_BUILD_TESTS_OPTION_STR} OFF)

if(KUKU_BUILD_TESTS)
    set(KUKU_THIRDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty)
    set(THIRDPARTY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)
    kuku_fetch_thirdparty_content(ExternalGTest)
    add_library(GTest::gtest ALIAS gtest)

    add_executable(kukutest)
    add_subdirectory(tests/kuku)
    target_link_libraries(kukutest PRIVATE ${KUKU_LIBRARY_NAME} GTest::gtest)
endif()

#######################################
# Configure KukuNet and NuGet package #
#######################################

# In Windows we will set the Kuku_C library path according to the CMake generator
set(KUKU_WINDOWS_KUKU_C_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
get_property(KUKU_IS_MULTI_CONFIG_GENERATOR GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(KUKU_IS_MULTI_CONFIG_GENERATOR)
    # Is this the Visual Studio generator? If so, the output path will contain the configuration.
    # We shall use CMAKE_BUILD_TYPE here, which by default will be "Release". The user has the
    # option of changing this by explicitly specifying CMAKE_BUILD_TYPE, which multi-config
    # generators otherwise ignore.
    set(KUKU_WINDOWS_KUKU_C_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
endif()

# Create KukuNet.csproj, KukuNetExamples.csproj and KukuNetTest.csproj
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/src/KukuNet.csproj.in
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/src/KukuNet.csproj
    @ONLY)
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/tests/KukuNetTests.csproj.in
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/tests/KukuNetTests.csproj
    @ONLY)
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/examples/KukuNetExamples.csproj.in
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/examples/KukuNetExamples.csproj
    @ONLY)

# Set the kukuc dynamic library file names to be included in creating
# the NuGet package. When building a multi-platform NuGet package, the
# dynamic library paths need to be specified explicitly in the NuGet
# command. See dotnet/nuget/KukuNet.nuspec.in.

# Create KukuNet-multi.nuspec for a multi-platform NuGet package
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/nuget/KukuNet-multi.nuspec.in
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/nuget/KukuNet-multi.nuspec
    @ONLY)

set(NUGET_WINDOWS_KUKU_C_PATH "")
set(NUGET_LINUX_KUKU_C_PATH "")
set(NUGET_MACOS_KUKU_C_PATH "")

# Supporting local building of NuGet package
if(WIN32)
    set(NUGET_WINDOWS_KUKU_C_PATH ${KUKU_WINDOWS_KUKU_C_DIRECTORY}/kukuc.dll)
elseif(UNIX AND NOT APPLE)
    set(NUGET_LINUX_KUKU_C_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libkukuc.so)
elseif(APPLE)
    set(NUGET_MACOS_KUKU_C_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libkukuc.dylib)
endif()

# Create KukuNet.nuspec for a local NuGet pack from KukuNet.nuspec.in
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/nuget/KukuNet.nuspec.in
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/nuget/KukuNet.nuspec
    @ONLY)
