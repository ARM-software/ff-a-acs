#!/bin/bash
#-------------------------------------------------------------------------------
# Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set -e  # Exit on error
JOBS=$(nproc --ignore=2)

# --------------------------------------------------------------------------------------------------
# Helpers

# Function to check if a tool exists
check_tool_exists() {
    if command -v "$1" >/dev/null 2>&1; then
        echo "$1 is installed."
        return 0
    else
        echo "$1 is not installed, please install the missing toolchain and retry"
        exit 0
    fi
}

# Function to check if dependency exists
check_dependency() {
    local path="$1"
    if [ ! -e "$path" ]; then
        echo "Error: Dependency not found at $path. Exiting."
        exit 1
    fi
}

# --------------------------------------------------------------------------------------------------
# Dependency
INPUT_DTS=$WORKSPACE/configs/dts/dt.dts
OUTPUT_DTB_DIR=$WORKSPACE/dt-build/
OUTPUT_DTB=dt.dtb

# Precompile Checks
if [ -z "${WORKSPACE}" ]; then
  echo "WORKSPACE is not set. Please set the variable and try again."
  exit 1
fi

check_dependency $INPUT_DTS;
check_tool_exists dtc;

# --------------------------------------------------------------------------------------------------
# Build

# Build kernel
mkdir -p "$OUTPUT_DTB_DIR"

# Compile the device tree
dtc -I dts -O dtb -o "$OUTPUT_DTB_DIR/$OUTPUT_DTB" "$INPUT_DTS"
echo "Compilation successful: $OUTPUT_DTB created."

rsync -av $OUTPUT_DTB_DIR/$OUTPUT_DTB $WORKSPACE/output/

