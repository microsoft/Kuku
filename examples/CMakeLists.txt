# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

cmake_minimum_required(VERSION 3.12)

project(KukuExamples VERSION 1.1.1 LANGUAGES CXX)

# Executable will be in ../bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)

add_executable(example example.cpp)

# Import Kuku
find_package(Kuku 1.1.1 EXACT REQUIRED
    # Providing a path so this can be built without installing Kuku
    PATHS ${KukuExamples_SOURCE_DIR}/../src/cmake
)

# Link Kuku
target_link_libraries(example Kuku::kuku)
