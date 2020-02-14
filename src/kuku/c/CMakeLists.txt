# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

# Source files in this directory
target_sources(kukuc PRIVATE
    # ${CMAKE_CURRENT_LIST_DIR}/dllmain.cpp
    ${CMAKE_CURRENT_LIST_DIR}/kuku_ref.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/pch.cpp
)

# Add header files for installation
install(
    FILES
        # ${CMAKE_CURRENT_LIST_DIR}/framework.h
        ${CMAKE_CURRENT_LIST_DIR}/kuku_ref.h
        # ${CMAKE_CURRENT_LIST_DIR}/pch.h
    DESTINATION
        ${KUKU_INCLUDES_INSTALL_DIR}/kuku/c
)
