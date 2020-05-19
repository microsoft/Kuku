#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

BASE_DIR=$(dirname "$0")
KUKU_ROOT_DIR=$BASE_DIR/../
shopt -s globstar
clang-format -i $KUKU_ROOT_DIR/src/**/*.h
clang-format -i $KUKU_ROOT_DIR/src/**/*.cpp