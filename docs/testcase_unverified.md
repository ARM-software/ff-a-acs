
# Test case unverified document

This file contains list of tests that are unverified
due to there is no platform available to validate the code.

**Note**
The release is tested using following hashes of trusted-firmware-a
and Hafnium repositories. There were local changes made to the Hafnium
to help in more test coverage. The patches for the same has been
sent for review.

## Example steps to run ff-a-acs tests on TF-A
```
mkdir -p work ; cd work

# Download and build ff-a-acs for tgt_tfa_fvp platform
git clone https://github.com/ARM-software/ff-a-acs.git
cd ff-a-acs ;
mkdir build ; cd build
cmake ../ -G"Unix Makefiles" -DCROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- -DTARGET=tgt_tfa_fvp -DPLATFORM_NS_HYPERVISOR_PRESENT=0
make
cd ../../

# Download and build Hafnium repository
git clone "https://review.trustedfirmware.org/hafnium/hafnium"
cd hafnium
git reset --hard 34bc0a103db1ba0b6648de68b74f05c882c8bba7
git submodule update --init
git fetch https://review.trustedfirmware.org/hafnium/hafnium refs/changes/06/10606/6 && git cherry-pick FETCH_HEAD
git fetch https://review.trustedfirmware.org/hafnium/hafnium refs/changes/93/10893/2 && git cherry-pick FETCH_HEAD
git fetch https://review.trustedfirmware.org/hafnium/hafnium refs/changes/88/10788/2 && git cherry-pick FETCH_HEAD
git fetch https://review.trustedfirmware.org/hafnium/hafnium refs/changes/59/10759/10 && git cherry-pick FETCH_HEAD
git fetch https://review.trustedfirmware.org/hafnium/hafnium refs/changes/32/10932/2 && git cherry-pick FETCH_HEAD
make PROJECT=reference
cd -

# Download trusted-firmware-a repository
git clone "https://review.trustedfirmware.org/TF-A/trusted-firmware-a"
cd trusted-firmware-a
git reset --hard 6ea1a75df34c9beed4609b84544d473b8d5690e7

# Build trusted-firmware-a and integrate software stack to run ff-a-acs
make PLAT=fvp BL33=../ff-a-acs/build/output/vm1.bin DEBUG=1 BL32=../hafnium/out/reference/secure_aem_v8a_fvp_clang/hafnium.bin all fip SP_LAYOUT_FILE=../ff-a-acs/platform/manifest/tgt_tfa_fvp/sp_layout.json ARM_SPMC_MANIFEST_DTS=../ff-a-acs/platform/manifest/tgt_tfa_fvp/fvp_spmc_manifest.dts CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- ARM_ARCH_MINOR=5 BRANCH_PROTECTION=1 CTX_INCLUDE_PAUTH_REGS=1 SPD=spmd CTX_INCLUDE_EL2_REGS=1
cd -

# Run ff-a-acs tests on FVP Model
FVP_Base_RevC-2xAEMv8A -C pctl.startup=0.0.0.0 -C cluster0.NUM_CORES=4 -C cluster1.NUM_CORES=4 -C bp.secure_memory=1 -C bp.secureflashloader.fname=trusted-firmware-a/build/fvp/debug/bl1.bin -C bp.flashloader0.fname=trusted-firmware-a/build/fvp/debug/fip.bin -C cluster0.has_arm_v8-5=1 -C cluster1.has_arm_v8-5=1 -C pci.pci_smmuv3.mmu.SMMU_AIDR=2 -C pci.pci_smmuv3.mmu.SMMU_IDR0=0x0046123B -C pci.pci_smmuv3.mmu.SMMU_IDR1=0x00600002 -C pci.pci_smmuv3.mmu.SMMU_IDR3=0x1714 -C pci.pci_smmuv3.mmu.SMMU_IDR5=0xFFFF0472 -C pci.pci_smmuv3.mmu.SMMU_S_IDR1=0xA0000002 -C pci.pci_smmuv3.mmu.SMMU_S_IDR2=0 -C pci.pci_smmuv3.mmu.SMMU_S_IDR3=0 -C cluster0.has_branch_target_exception=1 -C cluster1.has_branch_target_exception=1 -C cluster0.restriction_on_speculative_execution=2 -C cluster1.restriction_on_speculative_execution=2 -C bp.pl011_uart0.out_file=- -C bp.pl011_uart1.out_file=- -C bp.pl011_uart2.out_file=-
```
## List of unverified tests

- ffa_features
- ffa_direct_message_error
- ffa_direct_message_error1
- ffa_msg_send_error
- share_input_error_checks1
- share_ro_retrieve_rw_64_vmsp
- share_ro_retrieve_rw_64_spsp
- share_ro_retrieve_rw_64_vmvm
- share_lower_upper_boundary_64_vmsp
- share_lower_upper_boundary_64_vmvm
- share_lower_upper_boundary_64_spsp
- share_state_machine_1
- share_state_machine_4
- share_state_machine_6
- lend_mem_access_after_lend_64_vm
- lend_mem_access_after_lend_64_sp
- lend_retrieve_mem_access_64_vmvm
- lend_retrieve_mem_access_64_vmsp
- lend_retrieve_mem_access_64_spsp
- donate_lower_upper_boundary_64_spsp
- donate_lower_upper_boundary_64_vmvm
- relinquish_state_machine_4

## License

Arm FF-A ACS is distributed under BSD-3-Clause License.

--------------

*Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.*
