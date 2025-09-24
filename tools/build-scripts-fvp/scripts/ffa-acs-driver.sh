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
KBUILD_DIR="$WORKSPACE/linux-build/build"
LINUX_DIR="$WORKSPACE/linux-build/linux"
MODULE_DIR="$WORKSPACE/acs-build/ff-a-acs/linux-acs/ffa-acs-drv"
KERNEL_IMAGE="$KBUILD_DIR/arch/arm64/boot/Image"

BUILD_ROOT_IMAGE=$OUTPUT/rootfs.ext2
BUILDROOT_DIR="$WORKSPACE/buildroot-build/buildroot"
BUILDROOT_TGT_DIR="$WORKSPACE/buildroot-build/build/target"
BUILDROOT_BUILD_DIR="$WORKSPACE/buildroot-build/build"
BUILDROOT_OVERLAY_DIR="$WORKSPACE/buildroot-build/overlay"

BUSYBOX_TGT_DIR="$WORKSPACE/busybox-build/rootfs"
BUSYBOX_IMAGE=$OUTPUT/guest-rootfs.cpio

# --------------------------------------------------------------------------------------------------
# Build

# Check necessary files for building modules
check_dependency "$MODULE_DIR"
check_dependency "$KBUILD_DIR"
check_dependency "$KERNEL_IMAGE"

# Prepare kernel tree for external module builds
make -C "$LINUX_DIR" -j$JOBS CROSS_COMPILE="$TOOLCHAIN" ARCH=arm64 O="$KBUILD_DIR" modules_prepare
make -C "$LINUX_DIR" -j$JOBS CROSS_COMPILE="$TOOLCHAIN" ARCH=arm64 O="$KBUILD_DIR" modules

check_dependency "$KBUILD_DIR/Module.symvers"
check_dependency "$KBUILD_DIR/.config"
check_dependency "$KBUILD_DIR/include/generated/autoconf.h"

check_dependency "$BUILD_ROOT_IMAGE"
check_dependency "$BUILDROOT_DIR"
check_dependency "$BUILDROOT_TGT_DIR"
check_dependency "$BUILDROOT_BUILD_DIR"
check_dependency "$BUILDROOT_OVERLAY_DIR"

check_dependency "$BUSYBOX_TGT_DIR"
check_dependency "$BUSYBOX_IMAGE"

# --------------------------------------------------------------------------------------------------
# Build ffa-acs-user-driver Module
echo "[INFO] Building ffa-acs-user-driver module..."
make -C "$MODULE_DIR" CROSS_COMPILE="$TOOLCHAIN" KERNELDIR="$KBUILD_DIR" all -j"$JOBS"
echo "[SUCCESS] Kernel module built at: $MODULE_DIR/*.ko"

# --------------------------------------------------------------------------------------------------
# Install FF-A ACS Driver into Busybox
make -C "$LINUX_DIR" ARCH=arm64 CROSS_COMPILE="$TOOLCHAIN" O="$KBUILD_DIR" INSTALL_MOD_PATH="$BUSYBOX_TGT_DIR" modules_install
make -C "$LINUX_DIR" ARCH=arm64 CROSS_COMPILE="$TOOLCHAIN" O="$KBUILD_DIR" M="$MODULE_DIR" INSTALL_MOD_PATH="$BUSYBOX_TGT_DIR" modules_install
depmod -b $BUSYBOX_TGT_DIR $(make -s -C $LINUX_DIR O=$KBUILD_DIR kernelrelease)

pushd "$BUSYBOX_TGT_DIR"
find . | cpio -o --format=newc > "$BUSYBOX_IMAGE"
popd

# --------------------------------------------------------------------------------------------------
# Install FF-A ACS Driver into Buildroot
make -C "$LINUX_DIR" ARCH=arm64 CROSS_COMPILE="$TOOLCHAIN" O="$KBUILD_DIR" INSTALL_MOD_PATH="$BUILDROOT_TGT_DIR" modules_install
make -C $LINUX_DIR ARCH=arm64 CROSS_COMPILE=$TOOLCHAIN O=$KBUILD_DIR M="$MODULE_DIR" INSTALL_MOD_PATH=$BUILDROOT_TGT_DIR modules_install
depmod -b $BUILDROOT_TGT_DIR $(make -s -C $LINUX_DIR O=$KBUILD_DIR kernelrelease)

# Re install guest ramfs
install -m 644 $BUSYBOX_IMAGE $BUILDROOT_OVERLAY_DIR/xen-guests/linux/ramdisk.cpio
make -C $BUILDROOT_DIR O=$BUILDROOT_BUILD_DIR BR2_ROOTFS_OVERLAY=$BUILDROOT_OVERLAY_DIR all -j$JOBS

# --------------------------------------------------------------------------------------------------
# Optional: Copy result
mkdir -p "$WORKSPACE/output"
cp -f $BUILDROOT_BUILD_DIR/images/rootfs.ext2 $BUILD_ROOT_IMAGE
cp "$MODULE_DIR"/*.ko "$WORKSPACE/output/"
echo "[INFO] Module copied to output directory."
