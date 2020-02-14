# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

target_sources(kukutest
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/common.cpp
        ${CMAKE_CURRENT_LIST_DIR}/kuku.cpp
        ${CMAKE_CURRENT_LIST_DIR}/locfunc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/testrunner.cpp
)
