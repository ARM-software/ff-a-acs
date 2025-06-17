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
# Dependency
EXTBIN_DIR=$WORKSPACE/output
SHARE_DIR=$WORKSPACE/share

MODEL_BIN=FVP_Base_RevC-2xAEMvA
ACS_APP=acs-linux-app1.elf

BL1=$EXTBIN_DIR/bl1_spm_el$SPM_EL_LEVEL.bin
FIP=$EXTBIN_DIR/fip_spm_el$SPM_EL_LEVEL.bin
ROOTFS=$EXTBIN_DIR/rootfs.ext2
KERNEL=$EXTBIN_DIR/Image
DT=$EXTBIN_DIR/dt.dtb
XEN=$EXTBIN_DIR/xen

# Pre-Compile Checks
if [ -z "${WORKSPACE}" ]; then
  echo "WORKSPACE is not set. Please set the variable and try again."
  exit 1
fi

check_tool_exists $MODEL_BIN;
check_dependency $BL1 "BL1";
check_dependency $FIP "FIP";
check_dependency $ROOTFS "ROOTFS";
check_dependency $KERNEL "KERNEL";
check_dependency $DT "DT";
check_dependency $XEN "XEN";
check_dependency "$EXTBIN_DIR/$ACS_APP" "ACS_APP";


# --------------------------------------------------------------------------------------------------
# Run Setup
mkdir -p "$SHARE_DIR"

# Remove the file if it exists and copy latest user space acs app
[ -f "$SHARE_DIR/$ACS_APP" ] && rm "$SHARE_DIR/$ACS_APP"  
rsync -av $EXTBIN_DIR/$ACS_APP $SHARE_DIR/

# FVP Command
CMD_RUN="-C pctl.startup=0.0.0.0 \
         -C cluster0.NUM_CORES=4 \
         -C cluster1.NUM_CORES=4 \
         -C bp.secure_memory=1 \
         -C bp.secureflashloader.fname=$BL1 \
         -C bp.flashloader0.fname=$FIP \
         -C bp.pl011_uart0.out_file=- \
         -C bp.pl011_uart1.out_file=- \
         -C bp.pl011_uart2.out_file=- \
         -C cluster0.has_arm_v8-5=1 \
         -C cluster1.has_arm_v8-5=1 \
         -C cluster0.has_pointer_authentication=2 \
         -C cluster1.has_pointer_authentication=2 \
         -C cluster0.has_branch_target_exception=1 \
         -C cluster1.has_branch_target_exception=1 \
         -C cluster0.memory_tagging_support_level=2 \
         -C cluster1.memory_tagging_support_level=2 \
         -C bp.dram_metadata.is_enabled=1 \
         -C pci.pci_smmuv3.mmu.SMMU_AIDR=2 \
         -C pci.pci_smmuv3.mmu.SMMU_IDR0=0x0046123B \
         -C pci.pci_smmuv3.mmu.SMMU_IDR1=0x00600002 \
         -C pci.pci_smmuv3.mmu.SMMU_IDR3=0x1714 \
         -C pci.pci_smmuv3.mmu.SMMU_IDR5=0xFFFF0472 \
         -C pci.pci_smmuv3.mmu.SMMU_S_IDR1=0xA0000002 \
         -C pci.pci_smmuv3.mmu.SMMU_S_IDR2=0 \
         -C pci.pci_smmuv3.mmu.SMMU_S_IDR3=0 \
         -C cluster0.gicv3.extended-interrupt-range-support=1 \
	     -C cluster1.gicv3.extended-interrupt-range-support=1 \
	     -C gic_distributor.extended-ppi-count=64 \
	     -C gic_distributor.extended-spi-count=1024 \
	     -C gic_distributor.ARE-fixed-to-one=1 \
         -C bp.virtioblockdevice.image_path=$ROOTFS \
         --data cluster0.cpu0=$KERNEL@0x83000000 \
         --data cluster0.cpu0=$DT@0x81000000 \
         --data cluster0.cpu0=$XEN@0x86000000 \
         -C cache_state_modelled=0 \
         -C bp.virtio_rng.enabled=1"
#        -C TRACE.GenericTrace.trace-sources=verbose_commentary,smmu_initial_transaction,smmu_final_transaction,*.pci.pci_smmuv3.mmu.*.*,*.pci.smmulogger.*,*.pci.tbu0_pre_smmu_logger.*,FVP_Base_RevC_2xAEMv8A.pci.pci_smmuv3,smmu_poison_tw_data \
#        --plugin /arm/projectscratch/pd/sbsa/users/rahg01/tools/GenericTrace.so"

$MODEL_BIN $CMD_RUN | tee $WORKSPACE/log