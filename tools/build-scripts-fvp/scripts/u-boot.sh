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

check_tool_exists git;
check_tool_exists make;
check_tool_exists aarch64-none-linux-gnu-gcc;

# --------------------------------------------------------------------------------------------------
# Build Setup
REPO_URL="https://github.com/u-boot/u-boot.git"
REVISION="v2024.07"
TOOLCHAIN=aarch64-none-linux-gnu-

BUILD_DIR="$WORKSPACE/u-boot-build/build"
UBOOT_DIR="$WORKSPACE/u-boot-build/u-boot/"

# --------------------------------------------------------------------------------------------------
# Build

# Clone repository if needed
if [ ! -d "$UBOOT_DIR" ]; then
    git clone --depth 1 --branch "$REVISION" "$REPO_URL" "$UBOOT_DIR"
fi

# Set up build directory
mkdir -p "$BUILD_DIR"

if [ ! -f "$BUILD_DIR/.config" ]; then
    make -C "$UBOOT_DIR" -j$JOBS O="$BUILD_DIR" CROSS_COMPILE="$TOOLCHAIN" vexpress_fvp_defconfig

    # Configure U-Boot for XEN
    pushd "$UBOOT_DIR"
    ./scripts/config --file "$BUILD_DIR/.config" \
          --enable CONFIG_USE_BOOTARGS \
          --set-str CONFIG_BOOTARGS 'console=hvc0 earlycon=pl011,0x1c090000 root=/dev/vda' \
          --set-str CONFIG_BOOTCOMMAND 'setenv fdt_high 0xffffffffffffffff; booti 0x86000000 - 0x81000000' \
          --disable CONFIG_TOOLS_MKEFICAPSULE
    popd

    make -C "$UBOOT_DIR" -j$JOBS O="$BUILD_DIR" CROSS_COMPILE="$TOOLCHAIN" olddefconfig
fi

# Build U-Boot
make -C "$UBOOT_DIR" -j$JOBS O="$BUILD_DIR" CROSS_COMPILE="$TOOLCHAIN"

# --------------------------------------------------------------------------------------------------
# Post Process

echo "U-Boot built successfully. Artifact location: $BUILD_DIR/u-boot.bin"
rsync -av $BUILD_DIR/u-boot.bin $WORKSPACE/output/
