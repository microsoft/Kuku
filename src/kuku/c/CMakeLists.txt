# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

# Source files in this directory
target_sources(kukuc PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/kuku_ref.cpp
)

# Add header files for installation
install(
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/kuku_ref.h
    DESTINATION
        ${KUKU_INCLUDES_INSTALL_DIR}/kuku/c
)
