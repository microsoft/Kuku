# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

# Source files in this directory
set(KUKU_SOURCE_FILES ${KUKU_SOURCE_FILES}
    ${CMAKE_CURRENT_LIST_DIR}/internal/blake2b.c
    ${CMAKE_CURRENT_LIST_DIR}/internal/blake2xb.c
    ${CMAKE_CURRENT_LIST_DIR}/kuku.cpp
)

# Add header files for installation
install(
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/internal/blake2.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/blake2-impl.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/hash.h
    DESTINATION
        ${KUKU_INCLUDES_INSTALL_DIR}/kuku/internal
)

install(
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/common.h
        ${CMAKE_CURRENT_LIST_DIR}/kuku.h
        ${CMAKE_CURRENT_LIST_DIR}/locfunc.h
    DESTINATION
        ${KUKU_INCLUDES_INSTALL_DIR}/kuku
)

set(KUKU_SOURCE_FILES ${KUKU_SOURCE_FILES} PARENT_SCOPE)
