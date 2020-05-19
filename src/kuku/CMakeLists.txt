# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

# Source files in this directory
target_sources(kuku_obj
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/internal/blake2b.c
        ${CMAKE_CURRENT_LIST_DIR}/internal/blake2xb.c
        ${CMAKE_CURRENT_LIST_DIR}/kuku.cpp
)

# Create the config file
configure_file(${CMAKE_CURRENT_LIST_DIR}/internal/config.h.in ${CMAKE_CURRENT_LIST_DIR}/internal/config.h)

# Add header files for installation
install(
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/internal/blake2.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/blake2-impl.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/config.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/hash.h
        ${CMAKE_CURRENT_LIST_DIR}/common.h
        ${CMAKE_CURRENT_LIST_DIR}/kuku.h
        ${CMAKE_CURRENT_LIST_DIR}/locfunc.h
    DESTINATION
        ${KUKU_INCLUDES_INSTALL_DIR}/kuku
)
