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
CONFIG_FILE="$WORKSPACE/configs/linux/linux-xen.config"
#PATCH_DIR="$WORKSPACE/patches/linux"

# Precompile Checks
if [ -z "${WORKSPACE}" ]; then
  echo "WORKSPACE is not set. Please set the variable and try again."
  exit 1
fi

check_dependency $CONFIG_FILE;
#check_dependency $PATCH_DIR;
check_tool_exists git;
check_tool_exists make;
check_tool_exists aarch64-none-linux-gnu-gcc;

# --------------------------------------------------------------------------------------------------
# Build Setup

REPO_URL="https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git"
BRANCH="v6.16"

BUILD_DIR="$WORKSPACE/linux-build/build"
LINUX_DIR="$WORKSPACE/linux-build/linux"

# --------------------------------------------------------------------------------------------------
# Build

# Clone repository if needed
if [ ! -d "$LINUX_DIR" ]; then
    git clone --depth=1 -b "$BRANCH" "$REPO_URL" "$LINUX_DIR"
fi

# Build kernel
mkdir -p "$BUILD_DIR"
if [ ! -f "$BUILD_DIR/.config" ]; then
    cp -f "$CONFIG_FILE" "$BUILD_DIR/.config"
    make -C "$LINUX_DIR" CROSS_COMPILE="$TOOLCHAIN" ARCH=arm64 O="$BUILD_DIR" olddefconfig -j$JOBS
fi
make -C "$LINUX_DIR" CROSS_COMPILE="$TOOLCHAIN" ARCH=arm64 O="$BUILD_DIR" Image -j$JOBS 
echo "Linux Kernel build completed successfully!"

# --------------------------------------------------------------------------------------------------
# Post Process
rsync -av $BUILD_DIR/arch/arm64/boot/Image $WORKSPACE/output/

