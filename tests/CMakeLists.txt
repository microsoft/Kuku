# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

cmake_minimum_required(VERSION 3.12)

project(KukuTest LANGUAGES CXX)

# Executable will be in ../bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)

add_executable(kukutest)

# Import Kuku
find_package(Kuku 1.1.1 EXACT REQUIRED
    # Providing a path so this can be built without installing Kuku
    PATHS ${KukuTest_SOURCE_DIR}/../src/cmake
)

# Add source files
add_subdirectory(kuku)

# Only build GTest
option(BUILD_GMOCK OFF)
option(INSTALL_GTEST OFF)
mark_as_advanced(BUILD_GMOCK INSTALL_GTEST)

# Add GTest
set(GTEST_DIR ${KukuTest_SOURCE_DIR}/thirdparty/googletest)
if(NOT EXISTS ${GTEST_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "Could not find `${GTEST_DIR}/CMakeLists.txt`. Run `git submodule update --init` and retry.")
endif()

add_subdirectory(${GTEST_DIR})

# Link Kuku and GTest
target_link_libraries(kukutest Kuku::kuku gtest)
