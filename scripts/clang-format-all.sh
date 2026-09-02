#!/bin/bash

# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

BASE_DIR=$(dirname "$0")
KUKU_ROOT_DIR=$BASE_DIR/..
shopt -s globstar nullglob

# Format Kuku's own sources only. third_party/ holds vendored BLAKE2 sources that
# are kept byte-identical to upstream and must never be reformatted.
#
# .c is included as well as .h/.cpp: Kuku's own C sources are few, but they exist -
# tests/kuku/c/c_linkage.c is compiled as C on purpose - and leaving the extension out
# would silently exempt them from the project's formatting.
#
# nullglob matters because no directory below holds all three extensions. Without it
# an unmatched pattern is passed through literally and clang-format fails trying to
# open a file called, for example, examples/**/*.h.
for dir in src tests examples; do
    files=("$KUKU_ROOT_DIR/$dir"/**/*.h "$KUKU_ROOT_DIR/$dir"/**/*.cpp "$KUKU_ROOT_DIR/$dir"/**/*.c)
    if [ ${#files[@]} -gt 0 ]; then
        clang-format -i "${files[@]}"
    fi
done
