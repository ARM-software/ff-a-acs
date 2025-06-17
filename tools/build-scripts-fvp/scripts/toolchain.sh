#!/bin/bash
#-------------------------------------------------------------------------------
# Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set -e  # Exit on any error

# --------------------------------------------------------------------------------------------------
# User Defined

# Required dependencies
REQUIRED_PACKAGES=(make libssl-dev flex bison python3 python3-serial python3-pip device-tree-compiler)
MISSING_PACKAGES=()

# Check for missing packages
for pkg in "${REQUIRED_PACKAGES[@]}"; do
    dpkg -s "$pkg" &>/dev/null || MISSING_PACKAGES+=("$pkg")
done

# Install missing packages if necessary
if [ ${#MISSING_PACKAGES[@]} -ne 0 ]; then
    echo "Missing packages: ${MISSING_PACKAGES[*]}"
    read -p "Install them now? (y/n) " choice
    [[ "$choice" =~ ^[Yy]$ ]] && sudo apt update && sudo apt install -y "${MISSING_PACKAGES[@]}" \
    || { echo "Missing dependencies. Exiting."; exit 1; }
fi

# Install Python package
pip3 install --user fdt

# --------------------------------------------------------------------------------------------------
# Validate
if [ -z "${WORKSPACE}" ]; then
  echo "WORKSPACE is not set. Please set the variable and try again."
  exit 1
fi

# Set up toolchain directory
TOOLCHAIN_BASE_DIR=$WORKSPACE/toolchains
mkdir -p "$TOOLCHAIN_BASE_DIR"
cd "$TOOLCHAIN_BASE_DIR"

# --------------------------------------------------------------------------------------------------
# Download and extract function using curl
download_and_extract() {
    local url=$1
    local archive=$(basename "$url")

    # Check if the file already exists
    if [ -f "$archive" ]; then
        echo "$archive already exists. Skipping download."
    else
        echo "Downloading $archive..."
        curl -LO "$url" && tar -xf "$archive"
    fi
}
# --------------------------------------------------------------------------------------------------
# Arm FVP
ARM_FVP_URL="https://developer.arm.com/-/cdn-downloads/permalink/FVPs-Architecture/FM-11.27/FVP_Base_RevC-2xAEMvA_11.27_19_Linux64.tgz"
download_and_extract "$ARM_FVP_URL"
export PATH=$TOOLCHAIN_BASE_DIR/Base_RevC_AEMvA_pkg/models/Linux64_GCC-9.3:$PATH
FVP_Base_RevC-2xAEMvA --version
echo "Verifying FVP installation..."
command -v FVP_Base_RevC-2xAEMvA &>/dev/null || { echo "FVP installation failed."; exit 1; }
echo "FVP_Base_RevC-2xAEMvA installed successfully."

# --------------------------------------------------------------------------------------------------
# Arm GNU Toolchain
ARM_TOOLCHAIN_VERSION="13.2.Rel1"
ARM_TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_TOOLCHAIN_VERSION}/binrel/arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-x86_64-aarch64-none-linux-gnu.tar.xz"
download_and_extract "$ARM_TOOLCHAIN_URL"
export PATH=$TOOLCHAIN_BASE_DIR/arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-x86_64-aarch64-none-linux-gnu/bin:$PATH
aarch64-none-linux-gnu-gcc --version;
echo "Verifying Arm GNU Toolchain installation..."
command -v aarch64-none-linux-gnu-gcc &>/dev/null || { echo "Arm GNU Toolchain installation failed."; exit 1; }
echo "Arm GNU Toolchain installed successfully."

# LLVM Clang
LLVM_VERSION="18.1.8"
LLVM_URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-18.04.tar.xz"
download_and_extract "$LLVM_URL"
export PATH=$TOOLCHAIN_BASE_DIR/clang+llvm-${LLVM_VERSION}-x86_64-linux-gnu-ubuntu-18.04/bin:$PATH
clang --version;
echo "Verifying LLVM Clang installation..."
command -v clang &>/dev/null || { echo "LLVM Clang installation failed."; exit 1; }
echo "LLVM Clang installed successfully."

echo "Setup completed successfully."
cd $WORKSPACE
