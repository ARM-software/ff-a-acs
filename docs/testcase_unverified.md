
# Test case unverified document

This file contains list of tests that are unverified
due to there is no platform available to validate the code.

**Note**
The release is tested using following hashes of trusted-firmware-a
and Hafnium repositories.

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
setenv PATH $PWD/prebuilts/linux-x64/clang/bin:$PWD/prebuilts/linux-x64/dtc:$PATH
git reset --hard 035fa15eb062593d43ce332ce698c6a073885203
git submodule update --init
make PROJECT=reference
cd -

# Download trusted-firmware-a repository
git clone "https://review.trustedfirmware.org/TF-A/trusted-firmware-a"
cd trusted-firmware-a
git reset --hard 6367d196aa9ec0ebbcbaa60d458cce530503ffad

# Build trusted-firmware-a and integrate software stack to run ff-a-acs
make SPD=spmd ARM_ARCH_MINOR=5 BRANCH_PROTECTION=1 CTX_INCLUDE_MTE_REGS=1 CTX_INCLUDE_EL2_REGS=1 CTX_INCLUDE_PAUTH_REGS=1 PLAT=fvp BL33=<path to ff-a-acs>/build/output/vm1.bin DEBUG=1 BL32=<path to hafnium>/out/reference/secure_aem_v8a_fvp_clang/hafnium.bin all ARM_SPMC_MANIFEST_DTS=<path to ff-a-acs>/platform/manifest/tgt_tfa_fvp/fvp_spmc_manifest.dts ARM_BL2_SP_LIST_DTS=<path to ff-a-acs>/platform/manifest/tgt_tfa_fvp/fvp_tb_fw_config.dts fip SP_LAYOUT_FILE=<path to ff-a-acs>/platform/manifest/tgt_tfa_fvp/sp_layout.json CROSS_COMPILE=/path-to-aarch64-gcc>/bin/aarch64-none-elf-
cd -

# Run ff-a-acs tests on FVP Model
FVP_Base_RevC-2xAEMv8A -C pctl.startup=0.0.0.0 -C cluster0.NUM_CORES=4 -C cluster1.NUM_CORES=4 -C bp.secure_memory=1 -C bp.secureflashloader.fname=trusted-firmware-a/build/fvp/debug/bl1.bin -C bp.flashloader0.fname=trusted-firmware-a/build/fvp/debug/fip.bin -C bp.pl011_uart0.out_file=- -C bp.pl011_uart1.out_file=- -C bp.pl011_uart2.out_file=- -C cluster0.has_arm_v8-5=1 -C cluster1.has_arm_v8-5=1 -C cluster0.has_pointer_authentication=2 -C cluster1.has_pointer_authentication=2 -C cluster0.has_branch_target_exception=1 -C cluster1.has_branch_target_exception=1 -C cluster0.memory_tagging_support_level=2 -C cluster1.memory_tagging_support_level=2 -C bp.dram_metadata.is_enabled=1 -C pci.pci_smmuv3.mmu.SMMU_AIDR=2 -C pci.pci_smmuv3.mmu.SMMU_IDR0=0x0046123B -C pci.pci_smmuv3.mmu.SMMU_IDR1=0x00600002 -C pci.pci_smmuv3.mmu.SMMU_IDR3=0x1714  -C pci.pci_smmuv3.mmu.SMMU_IDR5=0xFFFF0472 -C pci.pci_smmuv3.mmu.SMMU_S_IDR1=0xA0000002 -C pci.pci_smmuv3.mmu.SMMU_S_IDR2=0 -C pci.pci_smmuv3.mmu.SMMU_S_IDR3=0
```
## List of unverified tests

- lend_sepid
- share_sepid
- donate_sepid
- share_sepid_multiple_borrowers
- share_sepid_multiple_borrowers

## List of skipped tests

- ffa_direct_message_error
- ffa_direct_message_error1
- ffa_msg_send
- ffa_msg_send_error
- share_ro_retrieve_rw_64_vmvm
- share_ro_retrieve_rw_32_vmvm
- share_lower_upper_boundary_64_vmvm
- share_lower_upper_boundary_32_vmvm
- share_lower_upper_boundary_64_vmsp
- share_lower_upper_boundary_64_spsp
- lend_mem_access_after_lend_64_vm
- lend_mem_access_after_lend_64_sp
- share_ro_retrieve_rw_64_vmsp
- share_ro_retrieve_rw_64_spsp
- lend_retrieve_mem_access_32_vmvm
- lend_retrieve_mem_access_64_vmvm
- lend_retrieve_mem_access_64_vmsp
- lend_retrieve_mem_access_64_spsp
- lend_lower_upper_boundary_32_vmvm
- lend_lower_upper_boundary_64_vmsp
- lend_lower_upper_boundary_64_spsp
- lend_lower_upper_boundary_64_vmvm
- donate_lower_upper_boundary_32_vmvm
- donate_lower_upper_boundary_64_spsp
- donate_lower_upper_boundary_64_vmvm
- relinquish_mem_unmap_check_vmvm
- donate_mem_access_after_donate_64_vm
- donate_mem_access_after_donate_64_sp
- donate_mem_access_after_donate_32_vm
- lend_mem_access_after_lend_32_vm

## List of failed tests

- mp_execution_contexts
- up_migrate_capable
- ffa_run
- lend_retrieve_mem_access_32_vmsp
- lend_cache_attr
- lend_cache_attr1
- lend_shareability_attr
- lend_shareability_attr1
- lend_shareability_attr2
- lend_shareability_attr3
- donate_state_machine_5
- donate_state_machine_3
- lend_state_machine_6
- lend_state_machine_4
- lend_state_machine_1
- relinquish_state_machine_3
- relinquish_state_machine_4
- lend_device_attr
- lend_device_attr1
- lend_retrieve_input_checks10
- lend_retrieve_input_checks9
- lend_retrieve_input_checks7
- lend_retrieve_input_checks2
- share_state_machine_6
- share_state_machine_4
- share_cache_attr
- share_shareability_attr
- share_shareability_attr1
- share_device_attr
- share_device_attr1
- share_state_machine_1
- share_input_error_checks1
- share_input_error_checks2
- share_retrieve_input_checks
- share_multiple_retrievals
- lend_retrieve_input_checks5
- donate_retrieve_input_checks5
- lend_retrieve_input_checks4
- donate_retrieve_input_checks3
- donate_retrieve_align_hint_check
- donate_retrieve_with_address_range
- donate_input_error_checks
- donate_input_error_checks1
- donate_input_error_checks2
- lend_multiple_retrievals
- donate_input_error_checks4
- donate_retrieve_input_checks1
- lend_retrieve_mem_access_32_spsp
- lend_retrieve_input_checks8
- lend_retrieve_input_checks
- lend_input_error_checks1
- lend_input_error_checks4
- sp_to_sp_blocked
- vm_to_sp_preempt

## License

Arm FF-A ACS is distributed under BSD-3-Clause License.

--------------

*Copyright (c) 2021-2022, Arm Limited or its affiliates. All rights reserved.*
