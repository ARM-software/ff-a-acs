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
# Dependency for UBOOT (NSEL2) + ACS + Hafnium (SEL2)
TOOLCHAIN=aarch64-none-linux-gnu-

EXTBINDIR="$WORKSPACE/output"
BL33="$EXTBINDIR/u-boot.bin"
if [ "$SPM_EL_LEVEL" = "2" ]; then
    BL32="$EXTBINDIR/hafnium.bin"
    BUILD_TYPE=debug
    SPMC_MANIFEST_DTS="$EXTBINDIR/fvp_spmc_manifest_linux_xen.dts"
elif [ "$SPM_EL_LEVEL" = "3" ]; then
    BL32="$EXTBINDIR/sp1.bin"
    BUILD_TYPE=release
    SPMC_MANIFEST_DTS="$EXTBINDIR/v12/spmc_el3/sp1.dts"
else
  echo "Error: SPM_EL_LEVEL must be either '2' or '3'."
  exit 1
fi

SP_LAYOUT="$EXTBINDIR/sp_layout.json"

# --------------------------------------------------------------------------------------------------
# Build Set Up
if [ -z "${WORKSPACE}" ]; then
  echo "WORKSPACE is not set. Please set the variable and try again."
  exit 1
fi

check_tool_exists git;
check_tool_exists make;
check_tool_exists aarch64-none-linux-gnu-gcc;
check_dependency "$BL33" "BL33";
check_dependency "$BL32" "BL32";
check_dependency "$SPMC_MANIFEST_DTS" "SPMC_MANIFEST_DTS";
check_dependency "$SP_LAYOUT" "SP_LAYOUT";

# --------------------------------------------------------------------------------------------------
# Build Set Up
REPO_URL="https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git"
REVISION="v2.13.0"

BUILD_DIR="$WORKSPACE/tfa-build/trusted-firmware-a/build_spm_el$SPM_EL_LEVEL"
TFA_DIR="$WORKSPACE/tfa-build/trusted-firmware-a"

CMD_SPM_EL3="SPD=spmd \
              BL33=$BL33 \
              BL32=$BL32 \
              SPMD_SPM_AT_SEL2=0 \
              SPMC_AT_EL3=1 \
              ARM_SPMC_MANIFEST_DTS=$SPMC_MANIFEST_DTS \
              CROSS_COMPILE=$CROSS_COMPILE \
              PLAT=fvp \
              fip all"

# Build Command alias
CMD_SPM_EL2="SPD=spmd \
              ARM_ARCH_MAJOR=8 \
              ARM_ARCH_MINOR=5 \
              BRANCH_PROTECTION=1 \
              ENABLE_FEAT_MTE2=1 \
              PLAT_TEST_SPM=1 \
              GIC_EXT_INTID=1 \
              BL33=$BL33 \
              DEBUG=1 \
              BL32=$BL32 \
              ARM_SPMC_MANIFEST_DTS=$SPMC_MANIFEST_DTS \
              ARM_BL2_SP_LIST_DTS=$BUILD_DIR/fvp/debug/sp_list_fragment.dts \
              SP_LAYOUT_FILE=$SP_LAYOUT \
              CROSS_COMPILE=$TOOLCHAIN \
              PLAT=fvp \
              POETRY= \
              fip all"

# --------------------------------------------------------------------------------------------------
# Build

# Clone repository if needed
if [ ! -d "$TFA_DIR" ]; then
    git clone --depth 1 --branch "$REVISION" "$REPO_URL" "$TFA_DIR"
fi


# Dispatch based on SPM_EL_LEVEL

make -C "$TFA_DIR" realclean BUILD_BASE="$BUILD_BASE"
if [ "$SPM_EL_LEVEL" = "2" ]; then
    echo "Building for SPM at EL2..."
    make -C "$TFA_DIR" $CMD_SPM_EL2 BUILD_BASE="$BUILD_DIR" -j$JOBS
elif [ "$SPM_EL_LEVEL" = "3" ]; then
    echo "Building for SPM at EL3..."
    make -C "$TFA_DIR" $CMD_SPM_EL3 BUILD_BASE="$BUILD_DIR" -j$JOBS
else
  echo "Error: SPM_EL_LEVEL must be either '2' or '3'."
  exit 1
fi

echo "TF-A built successfully. Artifact location: $BUILD_DIR/fvp/debug/*"

# --------------------------------------------------------------------------------------------------
# Post Process

echo "Copying files for SPM_EL_LEVEL=$SPM_EL_LEVEL from: $BUILD_DIR/fvp/release"
for f in "$BUILD_DIR/fvp/$BUILD_TYPE/"*; do
    if [[ -f "$f" ]]; then
        base=$(basename "$f")
        ext="${base##*.}"
        name="${base%.*}"

        if [[ "$base" == "$ext" ]]; then
            # No extension
            new="${base}_spm_el$SPM_EL_LEVEL"
        else
            new="${name}_spm_el$SPM_EL_LEVEL.${ext}"
        fi

        cp "$f" "$WORKSPACE/output/$new"
    fi
done
