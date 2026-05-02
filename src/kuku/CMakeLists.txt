# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

set(KUKU_BLAKE2_DIR ${PROJECT_SOURCE_DIR}/third_party/blake2)

# hash.h includes "kuku/internal/blake2.h", and the .c files do likewise. In the
# install tree those paths resolve under ${KUKU_INCLUDES_INSTALL_DIR}/kuku/internal.
# In the build tree we mirror the vendored headers from third_party/blake2/ into
# ${CMAKE_BINARY_DIR}/src/kuku/internal/ so the same #include paths work; that
# directory is already on the public include path (set by kuku_set_include_directories).
configure_file(${KUKU_BLAKE2_DIR}/blake2.h
    ${CMAKE_BINARY_DIR}/src/kuku/internal/blake2.h COPYONLY)
configure_file(${KUKU_BLAKE2_DIR}/blake2-impl.h
    ${CMAKE_BINARY_DIR}/src/kuku/internal/blake2-impl.h COPYONLY)

# Source files
set(KUKU_SOURCE_FILES ${KUKU_SOURCE_FILES}
    ${KUKU_BLAKE2_DIR}/blake2b.c
    ${KUKU_BLAKE2_DIR}/blake2xb.c
    ${CMAKE_CURRENT_LIST_DIR}/kuku.cpp
)

# Install vendored BLAKE2 headers under kuku/internal/ so installed hash.h can
# include "kuku/internal/blake2.h" without knowing it came from third_party/.
install(
    FILES
        ${KUKU_BLAKE2_DIR}/blake2.h
        ${KUKU_BLAKE2_DIR}/blake2-impl.h
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
