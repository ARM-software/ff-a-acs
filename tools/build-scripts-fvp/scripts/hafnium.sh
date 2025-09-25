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

# --------------------------------------------------------------------------------------------------
# Precompile Checks

if [ -z "${WORKSPACE}" ]; then
  echo "WORKSPACE is not set. Please set the variable and try again."
  exit 1
fi

if [ "$SPM_EL_LEVEL" = "3" ]; then
    echo "Skipping hafnium build: SPM_EL_LEVEL is 3"
    (return 0 2>/dev/null) || exit 0
fi

check_tool_exists git;
check_tool_exists ninja;
check_tool_exists clang;

# --------------------------------------------------------------------------------------------------
# Build Setup

REPO_URL="https://git.trustedfirmware.org/hafnium/hafnium.git"
REVISION="v2.13.0"

BUILDDIR="$WORKSPACE/hafnium-build/hafnium/out"
HAFNIUM_DIR="$WORKSPACE/hafnium-build/hafnium/"

# --------------------------------------------------------------------------------------------------
# Build

# Clone repository if needed
if [ ! -d "$HAFNIUM_DIR" ]; then
    git clone --depth 1 --branch "$REVISION" "$REPO_URL" "$HAFNIUM_DIR"
    export PATH=$PWD/prebuilts/linux-x64/dtc:$PATH;
    git -C $HAFNIUM_DIR submodule update --init;
fi

export PATH=$PWD/prebuilts/linux-x64/dtc:$PATH;
clang -v;
make -C $HAFNIUM_DIR PROJECT=reference PLATFORM=secure_aem_v8a_fvp_vhe -j$JOBS;
echo "Hafnium built successfully. Artifact location: $BUILDDIR/reference/secure_aem_v8a_fvp_vhe_clang/hafnium.bin"

# --------------------------------------------------------------------------------------------------
# Post process output to common place
rsync -av $BUILDDIR/reference/secure_aem_v8a_fvp_vhe_clang/hafnium.bin $WORKSPACE/output/

