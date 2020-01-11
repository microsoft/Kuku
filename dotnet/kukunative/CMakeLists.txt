# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

cmake_minimum_required(VERSION 3.12)

project(KukuNative VERSION 1.0.0 LANGUAGES CXX)

# Import Kuku
find_package(Kuku 1.0.0 EXACT REQUIRED
    # Providing a path so this can be built without installing Kuku
    PATHS ../../src/cmake
)

add_library(KukuNative SHARED)

target_include_directories(KukuNative PUBLIC ../../src/)

target_sources(KukuNative 
PUBLIC
#	${CMAKE_CURRENT_SOURCE_DIR}/dllmain.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/kuku_ref.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp	
)

set_target_properties(KukuNative 
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Link Kuku
target_link_libraries(KukuNative Kuku::kuku)
