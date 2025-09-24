#!/bin/bash
#-------------------------------------------------------------------------------
# Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# --------------------------------------------------------------------------------------------------
# Script Set Up
set -e  # Exit on error
JOBS=$(nproc --ignore=2)

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
check_tool_exists cmake;
check_tool_exists make;
check_tool_exists aarch64-none-linux-gnu-gcc;

# --------------------------------------------------------------------------------------------------
# Build Set Up
TOOLCHAIN=aarch64-none-linux-gnu-
FFA_BUILD=ff-a-acs

REPO_URL="https://github.com/ARM-software/ff-a-acs.git"
REVISION="main"
LINUX_REPO_URL="https://gitlab.arm.com/linux-arm/linux-acs.git"
LINUX_REVISION="master"

# Check if ACS_DIR_OVERRIDE is set, otherwise use default
if [ -n "$ACS_DIR_OVERRIDE" ]; then
    ACS_DIR="$ACS_DIR_OVERRIDE"
else
    ACS_DIR="$WORKSPACE/acs-build/$FFA_BUILD"
fi

# Update dependent directories accordingly
BUILD_DIR_LINUX="$ACS_DIR/build-linux"
BUILD_DIR_BAREMETAL="$ACS_DIR/build-baremetal"
MANIFEST_DIR="$ACS_DIR/platform/manifest/tgt_tfa_fvp"

# Modify here for manifest version switch
SPMC_MANIFEST_DTS="$MANIFEST_DIR/fvp_spmc_manifest_linux_xen.dts"
SP_LAYOUT="$MANIFEST_DIR/sp_layout_v12.json"
SP_MANIFEST="$MANIFEST_DIR/v12"

# Modify here for ACS configurations
CMD_ACS_LINUX="-DCROSS_COMPILE=$TOOLCHAIN \
               -DPLATFORM_NS_HYPERVISOR_PRESENT=1 \
               -DTARGET_LINUX=1\
               -DENABLE_BTI=ON \
               -DVERBOSE=1 \
               -DCMAKE_BUILD_TYPE=Debug \
               -DPLATFORM_FFA_V_1_0=0 \
               -DPLATFORM_FFA_V_1_1=0 \
               -DPLATFORM_FFA_V_1_2=0 \
               -DPLATFORM_FFA_V_ALL=1 \
               -DPLATFORM_FFA_V_MULTI=0 \
               -DPLATFORM_SPMC_EL=$SPM_EL_LEVEL \
               -DPLATFORM_SP_EL=1"

CMD_ACS_BAREMETAL="-DCROSS_COMPILE=$TOOLCHAIN \
                   -DPLATFORM_NS_HYPERVISOR_PRESENT=1 \
                   -DXEN_SUPPORT=1\
                   -DENABLE_BTI=ON \
                   -DVERBOSE=1 \
                   -DCMAKE_BUILD_TYPE=Debug \
                   -DPLATFORM_FFA_V_1_0=0 \
                   -DPLATFORM_FFA_V_1_1=0 \
                   -DPLATFORM_FFA_V_1_2=0 \
                   -DPLATFORM_FFA_V_ALL=1 \
                   -DPLATFORM_FFA_V_MULTI=0 \
                   -DPLATFORM_SPMC_EL=$SPM_EL_LEVEL \
                   -DPLATFORM_SP_EL=1"

# --------------------------------------------------------------------------------------------------
# Build

# Clone repository if needed
if [ ! -d "$ACS_DIR" ]; then
    git clone --branch "$REVISION" "$REPO_URL" "$ACS_DIR"
    cd $ACS_DIR;
    git clone --branch "$LINUX_REVISION" "$LINUX_REPO_URL"
    git submodule update --init;
    cd -;
fi

[ -d "$BUILD_DIR_LINUX" ] && rm -rf "$BUILD_DIR_LINUX";
mkdir -p "$BUILD_DIR_LINUX";

pushd $BUILD_DIR_LINUX;
cmake ../ -G"Unix Makefiles" $CMD_ACS_LINUX -DSUITE=all;
popd
make -C $BUILD_DIR_LINUX -j$JOBS

echo "FFA ACS Linux built successfully. Artifact location: $BUILD_DIR_LINUX/output/*"

[ -d "$BUILD_DIR_BAREMETAL" ] && rm -rf "$BUILD_DIR_BAREMETAL";
mkdir -p "$BUILD_DIR_BAREMETAL";

pushd $BUILD_DIR_BAREMETAL;
cmake ../ -G"Unix Makefiles" $CMD_ACS_BAREMETAL -DSUITE=all;
popd
make -C $BUILD_DIR_BAREMETAL -j$JOBS

echo "FFA ACS Baremetal built successfully. Artifact location: $BUILD_DIR_BAREMETAL/output/*"

# --------------------------------------------------------------------------------------------------
# Post Process Outputs

# Copy Binaries
rsync -av --exclude='*/' $BUILD_DIR_LINUX/output/ $WORKSPACE/output/
rsync -av $BUILD_DIR_BAREMETAL/output/vm1.bin $WORKSPACE/output/

# Copy SPMC Manifest
rsync -av $SPMC_MANIFEST_DTS $WORKSPACE/output/

# Copy SP Layout Json
cat $SP_LAYOUT | sed -e "s,\.\..*/output/,," > "$WORKSPACE/output/sp_layout.json"

# Copy SP Manifest
rsync -av $SP_MANIFEST $WORKSPACE/output/

