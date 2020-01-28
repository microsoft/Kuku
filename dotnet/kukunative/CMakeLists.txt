# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

cmake_minimum_required(VERSION 3.12)

project(KukuNative VERSION 2.0.0 LANGUAGES CXX C)

# Import Kuku
find_package(Kuku 2.0.0 EXACT REQUIRED
    # Providing a path so this can be built without installing Kuku
    PATHS ${KukuNative_SOURCE_DIR}/../../src/cmake
)

# Check that Kuku is build as position-independent code
if(KUKU_LIB_BUILD_TYPE STREQUAL "Static")
    message(FATAL_ERROR "Kuku is not built as position-independent code; please set KUKU_LIB_BUILD_TYPE to either \"Static_PIC\" or \"Shared\" and rebuild.")
endif()

# Build in Release mode by default; otherwise use selected option
set(KUKUNATIVE_DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${KUKUNATIVE_DEFAULT_BUILD_TYPE} CACHE
        STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS "Release" "Debug" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Build type (CMAKE_BUILD_TYPE): ${CMAKE_BUILD_TYPE}")

# Required files and directories
include(GNUInstallDirs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${KukuNative_SOURCE_DIR}/../lib)
set(CMAKE_LIBRARY_RUNTIME_DIRECTORY ${KukuNative_SOURCE_DIR}/../bin)

# Check that size_t is 64 bits
include(CheckTypeSize)
check_type_size("size_t" SIZET LANGUAGE C)
if(NOT SIZET EQUAL 8)
    message(FATAL_ERROR "Detected sizeof(size_t) == ${SIZET} but ${PROJECT_NAME} requires sizeof(size_t) == 8.")
endif()

# Create library but add no source files yet
add_library(kukunative SHARED "")

# Set SOVERSION for shared library
set_target_properties(kukunative PROPERTIES
    SOVERSION ${KukuNative_VERSION_MAJOR}.${KukuNative_VERSION_MINOR}
    VERSION ${KukuNative_VERSION})

# Link Kuku
target_link_libraries(kukunative Kuku::kuku)

target_sources(kukunative PUBLIC
#	${CMAKE_CURRENT_SOURCE_DIR}/dllmain.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/kuku_ref.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp
)

target_include_directories(kukunative
    PRIVATE $<BUILD_INTERFACE:${KukuNative_SOURCE_DIR}>)

# Install the native component of the .NET wrapper library
install(TARGETS kukunative
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
