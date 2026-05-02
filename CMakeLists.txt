# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

cmake_minimum_required(VERSION 3.25)

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

# Conditionally enable the vcpkg "tests" feature when building tests.
option(KUKU_BUILD_TESTS "Build C++ tests for Kuku" OFF)
if(KUKU_BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(Kuku VERSION 3.0.0 LANGUAGES CXX C)

########################
# Global configuration #
########################

include(CheckTypeSize)

# Extra modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Always build position-independent-code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Make the install target depend on the all target (required by vcpkg)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY OFF)

# In Debug mode, enable KUKU_DEBUG.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(KUKU_DEBUG ON)
else()
    set(KUKU_DEBUG OFF)
endif()
message(STATUS "Kuku debug mode: ${KUKU_DEBUG}")

# Enable security hardening compile and link flags (cross-platform).
# Applied as directory-scope options so they do NOT propagate to downstream
# consumers via the install/export interface.
option(KUKU_ENABLE_HARDENING "Enable security hardening compile and link flags." ON)

if(KUKU_ENABLE_HARDENING)
    if(MSVC)
        # Control Flow Guard.
        add_compile_options(/guard:cf)
        add_link_options(/guard:cf)
        # Spectre load-hardening (silently ignored by older toolsets).
        add_compile_options(/Qspectre)
        # Randomized base address; debug symbols help static analysis tools.
        add_link_options(/DYNAMICBASE /DEBUG)
    else()
        include(CheckCXXCompilerFlag)

        # Stack canaries.
        add_compile_options(-fstack-protector-strong)

        # Stack-clash protection (no-op on Apple Clang; gated for older
        # GCC/Clang versions that don't support the flag).
        if(NOT APPLE)
            check_cxx_compiler_flag(-fstack-clash-protection KUKU_HAS_STACK_CLASH_PROTECTION)
            if(KUKU_HAS_STACK_CLASH_PROTECTION)
                add_compile_options(-fstack-clash-protection)
            endif()
        endif()

        # Standard library hardening (bounds-checked containers, etc.).
        add_compile_definitions(
            $<$<NOT:$<CONFIG:Debug>>:_GLIBCXX_ASSERTIONS>
            $<$<NOT:$<CONFIG:Debug>>:_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST>
        )

        # Full RELRO, immediate binding, non-executable stack (ELF only).
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")
            add_link_options(-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack)
        endif()

        # Fortified libc functions (requires -O1+, skip on Apple).
        if(NOT APPLE)
            add_compile_options($<$<NOT:$<CONFIG:Debug>>:-U_FORTIFY_SOURCE>)
            add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=2>)
        endif()
    endif()
endif()

# Required files and directories
include(GNUInstallDirs)

# Runtime path setup. Apple: loader-relative entries so installed dylibs resolve
# siblings without depending on the install prefix. ELF: absolute install lib
# dir matching the configure-time prefix. _USE_LINK_PATH FALSE prevents leaking
# build-machine paths (vcpkg _deps/, etc.) into the installed artifact's RPATH.
if(APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path;@loader_path/../${CMAKE_INSTALL_LIBDIR}")
else()
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

# Source Tree
set(KUKU_INCLUDES_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
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
set(KUKU_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Kuku)
set(KUKU_INCLUDES_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})

##################
# Various macros #
##################

# Set the C++ language version
macro(kuku_set_language target)
    target_compile_features(${target} PUBLIC cxx_std_17)
endmacro()

# Set the VERSION property
macro(kuku_set_version target)
    set_target_properties(${target} PROPERTIES VERSION ${Kuku_VERSION})
endmacro()

# Set the SOVERSION property
macro(kuku_set_soversion target)
    set_target_properties(${target} PROPERTIES
        SOVERSION ${Kuku_VERSION_MAJOR}.${Kuku_VERSION_MINOR})
endmacro()

# Set include directories for build and install interfaces.
macro(kuku_set_include_directories target)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${KUKU_INCLUDES_DIR}>
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
    kuku_set_language(kuku)
    kuku_set_include_directories(kuku)
    kuku_set_version(kuku)
    kuku_install_target(kuku KukuTargets)
    set(KUKU_LIBRARY_NAME "kuku")

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

# Create the kukuc shared library; sources are added below.
if(KUKU_BUILD_KUKU_C)
    # Check that size_t is 8 bytes (CheckTypeSize is included at the top of this file).
    check_type_size("size_t" SIZET LANGUAGE C)
    if(NOT ${SIZET} EQUAL 8)
        unset(SIZET CACHE)
        unset(HAVE_SIZET CACHE)
        message(FATAL_ERROR "kukuc requires a 64-bit platform")
    endif()
    unset(SIZET CACHE)
    unset(HAVE_SIZET CACHE)

    if(BUILD_SHARED_LIBS)
        message(FATAL_ERROR "kukuc requires `BUILD_SHARED_LIBS=OFF`")
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

# KUKU_BUILD_TESTS option is declared above project() so VCPKG_MANIFEST_FEATURES
# can include "tests" before vcpkg runs.
if(KUKU_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)

    add_executable(kukutest)
    add_subdirectory(tests/kuku)
    target_link_libraries(kukutest PRIVATE ${KUKU_LIBRARY_NAME} GTest::gtest)
endif()

#######################################
# Configure KukuNet and NuGet package #
#######################################

# In Windows we will set the kukuc library path according to the CMake generator
set(KUKU_WINDOWS_KUKU_C_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
get_property(KUKU_IS_MULTI_CONFIG_GENERATOR GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(KUKU_IS_MULTI_CONFIG_GENERATOR)
    # Is this the Visual Studio generator? If so, the output path will contain the configuration.
    # We shall use CMAKE_BUILD_TYPE here, which by default will be "Release". The user has the
    # option of changing this by explicitly specifying CMAKE_BUILD_TYPE, which multi-config
    # generators otherwise ignore.
    set(KUKU_WINDOWS_KUKU_C_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
endif()

# Create KukuNet.csproj, KukuNetTests.csproj and KukuNetExamples.csproj
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
set(NUGET_MACOS_X64_KUKU_C_PATH "")
set(NUGET_MACOS_ARM64_KUKU_C_PATH "")

# Supporting local building of the (single-platform) NuGet package. Selects the
# native artifact for the current host/architecture; the multi-platform pack
# (KukuNet-multi.nuspec.in) is populated separately by CI from each platform's
# build artifacts.
if(WIN32)
    set(NUGET_WINDOWS_KUKU_C_PATH ${KUKU_WINDOWS_KUKU_C_DIRECTORY}/kukuc.dll)
elseif(UNIX AND NOT APPLE)
    set(NUGET_LINUX_KUKU_C_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libkukuc.so)
elseif(APPLE)
    if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        set(NUGET_MACOS_ARM64_KUKU_C_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libkukuc.dylib)
    else()
        set(NUGET_MACOS_X64_KUKU_C_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libkukuc.dylib)
    endif()
endif()

# Create KukuNet.nuspec for a local NuGet pack from KukuNet.nuspec.in
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/nuget/KukuNet.nuspec.in
    ${CMAKE_CURRENT_LIST_DIR}/dotnet/nuget/KukuNet.nuspec
    @ONLY)
