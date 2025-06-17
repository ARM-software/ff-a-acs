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
TOOLCHAIN=aarch64-none-linux-gnu-
CONFIG_FILE="$WORKSPACE/configs/xen/xen-base.config"

# Precompile Checks
if [ -z "${WORKSPACE}" ]; then
  echo "WORKSPACE is not set. Please set the variable and try again."
  exit 1
fi
check_dependency $CONFIG_FILE;
check_tool_exists git;
check_tool_exists make;
check_tool_exists aarch64-none-linux-gnu-gcc;

# --------------------------------------------------------------------------------------------------
# Build Setup
REPO_URL="https://gitlab.com/xen-project/people/bmarquis/xen-ffa-research.git"
BRANCH="ffa-virtio/v2/ffa-v1.2"

BUILD_DIR="$WORKSPACE/xen-build/build"
XEN_DIR="$WORKSPACE/xen-build/xen-ffa-research"

# --------------------------------------------------------------------------------------------------
# Build

# Clone the repository if not already cloned
if [ ! -d "$XEN_DIR" ]; then
    git clone --depth=1 -b "$BRANCH" "$REPO_URL" "$XEN_DIR"

    git -C "$XEN_DIR" fetch origin

    # Check if already on the correct branch
    CURRENT_BRANCH=$(git -C "$XEN_DIR" rev-parse --abbrev-ref HEAD)
    if [ "$CURRENT_BRANCH" != "$BRANCH" ]; then
        git -C "$XEN_DIR" checkout "$BRANCH"
    fi

fi

# Compile Xen
if [ ! -f $BUILD_DIR/.config ]; then
    mkdir -p $BUILD_DIR
    cp -f $CONFIG_FILE $BUILD_DIR/.config
    make -C $XEN_DIR/xen -j$JOBS O=$BUILD_DIR XEN_TARGET_ARCH=arm64 \
        CROSS_COMPILE="$TOOLCHAIN" olddefconfig
fi
make -C $XEN_DIR/xen -j$JOBS O=$BUILD_DIR XEN_TARGET_ARCH=arm64 \
    CROSS_COMPILE="$TOOLCHAIN"

echo "Xen build completed successfully! Artifact location: $XEN_DIR/xen/xen"

# --------------------------------------------------------------------------------------------------
# Post Process
rsync -av $BUILD_DIR/xen $WORKSPACE/output/

