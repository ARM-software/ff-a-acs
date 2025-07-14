/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_CONFIG_H_
#define _PAL_CONFIG_H_

/* Command line arguments to ACS cmake build */
#ifndef CMAKE_BUILD
#define VERBOSITY                       3
#define PLATFORM_NS_HYPERVISOR_PRESENT  1
#define PLATFORM_SPMC_EL                2
#define PLATFORM_SP_EL                  1
#define SUITE                          all
#endif

/* ACS test endpoint image is position independent and
 * can be loaded at any address. ACS test endpoint have been
 * tested using 4KB translation granule. However, it is possible
 * to configure MMU with any of supported translation granule
 * size 4k, 64k and 16k.
 * Also the image size must be eqaul to 1MB when 4KB translation
 * granule used. The image size requirement could be
 * more in the case of 16KB or 64KB translation granule.
 */
// Common macro for all test SPs image entry point offset
#define PLATFORM_SP_IMAGE_OFFSET 0x4000
// Common macro for all test VMs image entry point offset
#define PLATFORM_VM_IMAGE_OFFSET 0x0

#define PLATFORM_PAGE_SIZE 0x1000

#define PAGE_SIZE_4K        0x1000
#define PAGE_SIZE_16K       (4 * 0x1000)
#define PAGE_SIZE_64K       (16 * 0x1000)

/*
 * Retrieving of memory region by specifying address ranges.
 * this is about retrieving memory without handle value and
 * specifying the offset field in the endpoint memory access descriptor.
 * Supported values: 0(NOT_SUPPORTED), 1(SUPPORTED)
 */
#define PLATFORM_MEM_RETRIEVE_USING_ADDRESS_RANGES 0

/* INNER_SHAREBLE and OUTER_SHAREABLE flag support
 * Supported values: 0(NOT_SUPPORTED), 1(SUPPORTED)
 */
#define PLATFORM_OUTER_SHAREABLE_SUPPORT_ONLY 0
#define PLATFORM_INNER_SHAREABLE_SUPPORT_ONLY 1
#define PLATFORM_INNER_OUTER_SHAREABLE_SUPPORT 0

/*
 * Device Info.
 * The provided addresses must be in physical view of endpoint.
 * It means, address can be :
 *  - VA address for S-EL0 endpoint
 *  - IPA for S-EL1 endpoint if SPMC is at EL2 otherwise it must be PA
 *  - IPA for NS-EL1 endpoint if hyp component is implemented otherwise it must be PA
 *
 */

/* Non-secure UART assigned to VM1 - PL011_UART2_BASE */
#define PLATFORM_NS_UART_BASE    0x1c0b0000
#define PLATFORM_NS_UART_SIZE    0x10000

/* Secure UART assigned to SP1 - PL011_UART2_BASE */
#define PLATFORM_S_UART_BASE    0x1c0b0000
#define PLATFORM_S_UART_SIZE    0x10000

/* Non-volatile memory range assigned to SP1 */
#define PLATFORM_NVM_BASE    (0x80000000+0x2800000)
#define PLATFORM_NVM_SIZE    0x10000

/* Base address of watchdog assigned to SP1 */
#define PLATFORM_WDOG_BASE    0x1C0F0000 //(SP805)
#define PLATFORM_WDOG_SIZE    0x10000
#define PLATFORM_WDOG_LOAD_VALUE (0x3E7 * 100 * 1000) // 100sec
#define PLATFORM_WDOG_INTR 32

/* Base address of trusted watchdog (SP805) */
#define PLATFORM_SP805_TWDOG_BASE    0x2A490000
#define PLATFORM_TWDOG_SIZE          0x10000
#define PLATFORM_TWDOG_INTID         56

/* Base address of AP_REFCLK_CNTBase1 System Timer */
#define PLATFORM_AP_REFCLK_CNTBASE1     0x2A830000
#define PLATFORM_AP_REFCLK_SIZE         0x1000
#define PALTFORM_AP_REFCLK_CNTPSIRQ1    58

#define PLATFORM_NS_WD_BASE  0x2A440000
#define PLATFORM_NS_WD_SIZE  0x1000
#define PLATFORM_NS_WD_INTR  59

#define ARM_SP805_TWDG_CLK_HZ   32768

/*
 * Base address of read only memory region
 */
#define PLATFORM_MEM_READ_ONLY_BASE  0xfe300000
#define PLATFORM_MEM_READ_ONLY_SIZE  0x1000

/* SMMU upstream device memory region 16KB */
#define PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION 0x7800000
#define PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID 0x7700000

#define PLAT_SMMU_UPSTREAM_DEVICE_MEM_SIZE   0x10000

/* SMMU Test Engine Memory Map  */
#define PLAT_SMMUV3_TEST_ENGINE_MEM_REGION 0x2bfe0000
#define PLAT_SMMUV3_TEST_ENGINE_MEM_SIZE   0x20000

/* SMMU stream id */
#define PLATFORM_SMMU_STREAM_ID           1
#define PLATFORM_SMMU_STREAM_ID_INVALID   2

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
/* Base FVP compatible GIC memory map */
#define GICD_BASE       0x2f000000
#define GICR_BASE       0x2f100000
#define GICC_BASE       0x2c000000
#define GICD_SIZE       0x10000
#define GICR_SIZE       0x100000
#define GICC_SIZE       0x2000

/* Non-secure EL1 physical timer interrupt */
#define IRQ_PHY_TIMER_EL1           30
/* Non-secure EL1 virtual timer interrupt */
#define IRQ_VIRT_TIMER_EL1          27
/* Non-secure EL2 physical timer interrupt */
#define IRQ_PHY_TIMER_EL2           26

/*
 * Secure Partition manifest info.
 */

/* Test Partition IDs assigned by the system */
#define PLATFORM_SP1_ID             (1 | (1 << 15))
#define PLATFORM_SP2_ID             (2 | (1 << 15))
#define PLATFORM_SP3_ID             (3 | (1 << 15))
#define PLATFORM_SP4_ID             (4 | (1 << 15))

#define PLATFORM_PRIMARY_SCHEDULER_EL  1

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1)
    #if (PLATFORM_PRIMARY_SCHEDULER_EL == 2)
        #define PLATFORM_VM1_ID              1
        #define PLATFORM_VM2_ID              2
        #define PLATFORM_VM3_ID              3
    #elif (PLATFORM_PRIMARY_SCHEDULER_EL == 1)
        // Assuming Primary scheduler ID   = 1
        #define PLATFORM_VM1_ID              2
        #define PLATFORM_VM2_ID              3
        #define PLATFORM_VM3_ID              4
    #endif
#endif

/*
 * CPU info
 */
#define PLATFORM_NO_OF_CPUS 8

#ifndef __ASSEMBLER__
extern uint8_t pal_misc_buffer[256];
#define PLATFORM_SHARED_REGION_BASE (void*)pal_misc_buffer
#endif

#endif /* _PAL_CONFIG_H_ */
