#!/bin/bash
#-------------------------------------------------------------------------------
# Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set -e

# --------------------------------------------------------------------------------------------------
# Helpers

# Function to check if dependency exists and print the name of the variable
check_dependency() {
    local path="$1"
    local var_name="$2"  # The second argument is the name of the variable

    if [ ! -e "$path" ]; then
        echo "Error: Dependency '$var_name' not found at $path. Exiting."
        exit 1
    fi
}

# --------------------------------------------------------------------------------------------------
# Build Setup

# Ensure WORKSPACE and CONFIGS variables are defined
if [ -z "${WORKSPACE}" ]; then
  echo "WORKSPACE is not set. Please set the variable and try again."
  exit 1
fi

BUILD_DIR="$WORKSPACE/busybox-build/build"
BUSYBOX_DIR="$WORKSPACE/busybox-build/busybox-1.36.1"
ROOTFS_DIR="$WORKSPACE/busybox-build/rootfs"

# --------------------------------------------------------------------------------------------------
# Dependency
OUTPUT=$WORKSPACE/output
IMAGE=$OUTPUT/guest-rootfs.cpio
ACS_IMAGE=$OUTPUT/acs-linux-app1.elf
CONFIG_FILE="$WORKSPACE/configs/busybox/busybox.config"

check_dependency $ACS_IMAGE "ACS_IMAGE";
check_dependency $CONFIG_FILE "busybox.config";

# --------------------------------------------------------------------------------------------------
# Build
# Download and extract BusyBox
if [ ! -d $BUSYBOX_DIR ]; then
        mkdir -p $WORKSPACE/busybox-build
        wget https://busybox.net/downloads/busybox-1.36.1.tar.bz2 -O $WORKSPACE/busybox.tar.bz2
        tar -xjf $WORKSPACE/busybox.tar.bz2 -C $WORKSPACE/busybox-build
        rm -f $WORKSPACE/busybox.tar.bz2
fi

# Add config if none
if [ ! -f $BUILD_DIR/.config ]; then
    mkdir -p $BUILD_DIR
    cp -f ${CONFIG_FILE} $BUILD_DIR/.config
    yes "" | make -C $BUSYBOX_DIR O=$BUILD_DIR ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- oldconfig
fi

# Compile BusyBox with minimal configuration
make -C $BUSYBOX_DIR O=$BUILD_DIR -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

# Create necessary directories
rm -rf $ROOTFS_DIR
mkdir -p $ROOTFS_DIR/{bin,sbin,etc,proc,sys,dev,tmp,var,lib,usr/bin,usr/sbin}
chmod 1777 $ROOTFS_DIR/tmp

make -C $BUSYBOX_DIR O=$BUILD_DIR ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- install CONFIG_PREFIX=$ROOTFS_DIR

# Create init script
rm -f $ROOTFS_DIR/init
cat > $ROOTFS_DIR/init << EOF
#!/bin/sh
mount -t devtmpfs none /dev
mount -t proc proc /proc
mount -t sysfs sys /sys
exec /bin/sh
EOF
chmod a+x $ROOTFS_DIR/init

# install ACS Test Image
install -m 755 $ACS_IMAGE $ROOTFS_DIR/bin/.

# Create a cpio image
pushd $ROOTFS_DIR
find . | cpio -o --format=newc > $IMAGE
popd

echo "Guest RootFS build completed successfully!"

cd $WORKSPACE
