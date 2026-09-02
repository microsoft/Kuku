# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

target_sources(kukuctest
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/kuku.cpp
        # Compiled as C, not C++. It exists to make kuku/c/kuku.h prove on every build
        # that it is still parsable by a C compiler; see the comment at the top of the file.
        ${CMAKE_CURRENT_LIST_DIR}/c_linkage.c
        ${CMAKE_CURRENT_LIST_DIR}/../testrunner.cpp
)
