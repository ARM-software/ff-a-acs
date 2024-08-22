
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
cmake ../ -G"Unix Makefiles" -DCROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- -DTARGET=tgt_tfa_fvp -DPLATFORM_NS_HYPERVISOR_PRESENT=0 -DPLATFORM_FFA_V_ALL=1 -DPLATFORM_SP_EL=<1|0> -DENABLE_BTI=ON
make
cd ../../

# Download and build Hafnium repository
git clone "https://review.trustedfirmware.org/hafnium/hafnium"
cd hafnium
setenv PATH $PWD/prebuilts/linux-x64/clang/bin:$PWD/prebuilts/linux-x64/dtc:<path-to-clang>:$PATH
git reset --hard fd374b8c9227b34eed1e8c1f8b4488c9596ca0e5
git submodule update --init
git fetch https://review.trustedfirmware.org/hafnium/hafnium refs/changes/25/29025/5 && git cherry-pick FETCH_HEAD
make PROJECT=reference PLATFORM=secure_aem_v8a_fvp_vhe
cd -

# Download trusted-firmware-a repository
git clone "https://review.trustedfirmware.org/TF-A/trusted-firmware-a"
cd trusted-firmware-a
git reset --hard 9babc7c2d69c2b75395273e34b250aed03254e1e

# Build trusted-firmware-a and integrate S-EL1 patitions software stack to run ff-a-acs
make SPD=spmd ARM_ARCH_MINOR=5 GIC_EXT_INTID=1 BRANCH_PROTECTION=1 ENABLE_FEAT_MTE2=1 CTX_INCLUDE_EL2_REGS=1 CTX_INCLUDE_PAUTH_REGS=1 PLAT=fvp BL33=<path to ff-a-acs>/build/output/vm1.bin DEBUG=1 BL32=<path to hafnium>/out/reference/secure_aem_v8a_fvp_vhe_clang/hafnium.bin all ARM_SPMC_MANIFEST_DTS=<path to ff-a-acs>/platform/manifest/tgt_tfa_fvp/fvp_spmc_manifest.dts ARM_BL2_SP_LIST_DTS=<path to trusted-firmware-a>/build/fvp/debug/sp_list_fragment.dts fip SP_LAYOUT_FILE=<path to ff-a-acs>/platform/manifest/tgt_tfa_fvp/sp_layout.json CROSS_COMPILE=/path-to-aarch64-gcc>/bin/aarch64-none-elf-
cd -

# Build trusted-firmware-a and integrate S-EL0 patitions software stack to run ff-a-acs
make SPD=spmd ARM_ARCH_MINOR=5 GIC_EXT_INTID=1 BRANCH_PROTECTION=1 ENABLE_FEAT_MTE2=1 CTX_INCLUDE_EL2_REGS=1 CTX_INCLUDE_PAUTH_REGS=1 PLAT=fvp BL33=<path to ff-a-acs>/build/output/vm1.bin DEBUG=1 BL32=<path to hafnium>/out/reference/secure_aem_v8a_fvp_vhe_clang/hafnium.bin all ARM_SPMC_MANIFEST_DTS=<path to ff-a-acs>/platform/manifest/tgt_tfa_fvp/fvp_spmc_manifest_el0.dts ARM_BL2_SP_LIST_DTS=<path to trusted-firmware-a>/build/fvp/debug/sp_list_fragment.dts fip SP_LAYOUT_FILE=<path to ff-a-acs>/platform/manifest/tgt_tfa_fvp/sp_layout_el0.json CROSS_COMPILE=/path-to-aarch64-gcc>/bin/aarch64-none-elf-

# Run ff-a-acs tests on FVP Model
FVP_Base_RevC-2xAEMv8A -C pctl.startup=0.0.0.0 -C cluster0.NUM_CORES=4 -C cluster1.NUM_CORES=4 -C bp.secure_memory=1 -C bp.secureflashloader.fname=trusted-firmware-a/build/fvp/debug/bl1.bin -C bp.flashloader0.fname=trusted-firmware-a/build/fvp/debug/fip.bin -C bp.pl011_uart0.out_file=- -C bp.pl011_uart1.out_file=- -C bp.pl011_uart2.out_file=- -C cluster0.has_arm_v8-5=1 -C cluster1.has_arm_v8-5=1 -C cluster0.has_pointer_authentication=2 -C cluster1.has_pointer_authentication=2 -C cluster0.has_branch_target_exception=1 -C cluster1.has_branch_target_exception=1 -C cluster0.memory_tagging_support_level=2 -C cluster1.memory_tagging_support_level=2 -C bp.dram_metadata.is_enabled=1 -C pci.pci_smmuv3.mmu.SMMU_AIDR=2 -C pci.pci_smmuv3.mmu.SMMU_IDR0=0x0046123B -C pci.pci_smmuv3.mmu.SMMU_IDR1=0x00600002 -C pci.pci_smmuv3.mmu.SMMU_IDR3=0x1714 -C pci.pci_smmuv3.mmu.SMMU_IDR5=0xFFFF0472 -C pci.pci_smmuv3.mmu.SMMU_S_IDR1=0xA0000002 -C pci.pci_smmuv3.mmu.SMMU_S_IDR2=0 -C pci.pci_smmuv3.mmu.SMMU_S_IDR3=0 -C cluster0.gicv3.extended-interrupt-range-support=1 -C cluster1.gicv3.extended-interrupt-range-support=1 -C gic_distributor.extended-ppi-count=64 -C gic_distributor.extended-spi-count=1024 -C gic_distributor.ARE-fixed-to-one=1

```

## List of skipped tests

- ffa_direct_message_error
- ffa_direct_message_error1
- donate_lower_upper_boundary_32_vmvm
- donate_lower_upper_boundary_64_vmvm
- donate_mem_access_after_donate_32_vm
- donate_mem_access_after_donate_64_vm
- lend_lower_upper_boundary_32_vmvm
- lend_lower_upper_boundary_64_vmvm
- lend_mem_access_after_lend_32_vm
- lend_mem_access_after_lend_64_vm
- lend_retrieve_mem_access_32_vmvm
- lend_retrieve_mem_access_32_vmsp
- lend_retrieve_mem_access_64_vmvm
- lend_retrieve_mem_access_64_vmsp
- relinquish_mem_unmap_check_vmvm
- share_lower_upper_boundary_32_vmvm
- share_lower_upper_boundary_64_vmvm
- share_ro_retrieve_rw_64_vmvm
- share_ro_retrieve_rw_32_vmvm

## List of failed tests
- ffa_version
- ffa_features

## List of unverified tests

- ffa_run
- lend_retrieve_with_address_range
- share_retrieve_with_address_range
- lend_cache_attr
- lend_cache_attr1
- lend_shareability_attr
- lend_shareability_attr1
- lend_shareability_attr2
- lend_shareability_attr3
- lend_device_attr
- lend_device_attr1
- lend_cache_attr
- lend_cache_attr1
- lend_shareability_attr
- lend_shareability_attr1
- lend_shareability_attr2
- lend_shareability_attr3
- lend_retrieve_input_checks10
- lend_retrieve_input_checks7
- share_sepid_multiple_borrowers
- share_sepid_multiple_borrowers
- share_cache_attr
- share_shareability_attr
- share_shareability_attr1
- share_device_attr
- share_device_attr1
- lend_sepid
- share_sepid
- donate_sepid
- share_retrieve_input_checks
- donate_retrieve_input_checks5
- donate_retrieve_align_hint_check
- donate_retrieve_with_address_range
- donate_input_error_checks1
- mem_share_mmio
- mem_donate_mmio
- static_mapping_dma
- vm_to_up_sp_preempt
- vm_to_sp_managed_exit_4
- other_secure_int2
- other_secure_int7
- s_int_ec_blocked

## License

Arm FF-A ACS is distributed under BSD-3-Clause License.

--------------

*Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.*
