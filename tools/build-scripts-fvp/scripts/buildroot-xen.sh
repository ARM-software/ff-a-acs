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
OUTPUT=$WORKSPACE/output
IMAGE=$OUTPUT/rootfs.ext2

CONFIG_FILE="$WORKSPACE/configs/buildroot/buildroot-xen.config"
ACS_IMAGE=$OUTPUT/acs-linux-app.elf
GUEST_LINUX_ROOTFS=$OUTPUT/guest-rootfs.cpio
GUEST_LINUX_KERNEL=$OUTPUT/Image
GUEST_LINUX_CONFIG="$WORKSPACE/configs/xen-guest/linux-guest.cfg"
GUEST_BAREMETAL_KERNEL=$OUTPUT/vm1.bin
GUEST_BAREMETAL_CONFIG="$WORKSPACE/configs/xen-guest/baremetal-guest.cfg"

# Precompile Checks
if [ -z "${WORKSPACE}" ]; then
  echo "WORKSPACE is not set. Please set the variable and try again."
  exit 1
fi
check_dependency $CONFIG_FILE;
check_dependency $ACS_IMAGE;
check_dependency $GUEST_LINUX_ROOTFS;
check_dependency $GUEST_LINUX_KERNEL;
check_dependency $GUEST_LINUX_CONFIG;
check_dependency $GUEST_BAREMETAL_KERNEL;
check_dependency $GUEST_BAREMETAL_CONFIG;
check_tool_exists make;

# --------------------------------------------------------------------------------------------------
# Build Setup
REPO_URL="https://gitlab.com/buildroot.org/buildroot.git"
BRANCH="2024.08.x"

BUILD_DIR="$WORKSPACE/buildroot-build/build"
BUILDROOT_DIR="$WORKSPACE/buildroot-build/buildroot"
OVERLAY_DIR="$WORKSPACE/buildroot-build/overlay"

# --------------------------------------------------------------------------------------------------
# Build

# Clone the repository if not already cloned
if [ ! -d "$BUILDROOT_DIR" ]; then
    git clone --depth=1 -b "$BRANCH" "$REPO_URL" "$BUILDROOT_DIR"

    # Apply our patches
    git -C "$BUILDROOT_DIR" am $WORKSPACE/configs/buildroot/00*

fi

# Configure Buildroot
if [ ! -f $BUILD_DIR/.config ]; then
    mkdir -p $BUILD_DIR
    cp -f $CONFIG_FILE $BUILD_DIR/.config
    echo "XEN_OVERRIDE_SRCDIR = $WORKSPACE/xen-build/xen-ffa-research/" > $BUILD_DIR/local.mk
    make -C $BUILDROOT_DIR -j$JOBS O=$BUILD_DIR olddefconfig
fi

# Create buildroot overlay
mkdir -p $OVERLAY_DIR/bin
install -m 755 $ACS_IMAGE $OVERLAY_DIR/bin/.
mkdir -p $OVERLAY_DIR/xen-guests/linux
install -m 644 $GUEST_LINUX_ROOTFS $OVERLAY_DIR/xen-guests/linux/ramdisk.cpio
install -m 644 $GUEST_LINUX_KERNEL $OVERLAY_DIR/xen-guests/linux/kernel.bin
install -m 644 $GUEST_LINUX_CONFIG $OVERLAY_DIR/xen-guests/.
mkdir -p $OVERLAY_DIR/xen-guests/baremetal
install -m 644 $GUEST_BAREMETAL_KERNEL $OVERLAY_DIR/xen-guests/baremetal/kernel.bin
install -m 644 $GUEST_BAREMETAL_CONFIG $OVERLAY_DIR/xen-guests/.


# Compile buildroot
make -C $BUILDROOT_DIR O=$BUILD_DIR BR2_ROOTFS_OVERLAY=$OVERLAY_DIR all -j$JOBS

echo "Buildroot build completed successfully!"

# --------------------------------------------------------------------------------------------------
# Post Process
mkdir -p $OUTPUT
cp -f $BUILD_DIR/images/rootfs.ext2 $IMAGE

