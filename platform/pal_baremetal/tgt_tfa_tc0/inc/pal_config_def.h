/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
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
 * granule used. The image size requirement could be more in the case of 16KB or 64KB translation granule.
 * */
// Common macro for all test SPs image entry point offset
#define PLATFORM_SP_IMAGE_OFFSET 0x1000
// Common macro for all test VMs image entry point offset
#define PLATFORM_VM_IMAGE_OFFSET 0x0

#define PLATFORM_PAGE_SIZE 0x1000

#define PAGE_SIZE_4K        0x1000
#define PAGE_SIZE_16K       (4 * 0x1000)
#define PAGE_SIZE_64K       (16 * 0x1000)

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
#define PLATFORM_NS_UART_BASE    0x7FF80000
#define PLATFORM_NS_UART_SIZE    0x10000

/* Secure UART assigned to SP1 - PL011_UART2_BASE */
#define PLATFORM_S_UART_BASE    0x7FF80000
#define PLATFORM_S_UART_SIZE    0x10000

/* Non-volatile memory range assigned to SP1 */
#define PLATFORM_NVM_BASE    (0x80000000+0x0)
#define PLATFORM_NVM_SIZE    0x10000

/* Base address of watchdog assigned to SP1 */
#define PLATFORM_WDOG_BASE    0x1C0F0000 //(SP805)
#define PLATFORM_WDOG_SIZE    0x10000
#define PLATFORM_WDOG_LOAD_VALUE (0x3E7 * 10 * 1000) // 10sec
#define PLATFORM_WD_INTR_NUM 32

/*******************************************************************************
 * GIC-600 & interrupt handling related constants
 ******************************************************************************/
/* TC0 FVP compatible GIC memory map */
#define GICD_BASE       0x30000000
#define GICR_BASE       0x30140000
#define GICC_BASE       0x2C000000

/*
 * Secure Partition manifest info.
 */

/* Test Partition IDs assigned by the system */
#define PLATFORM_SP1_ID             (1 | (1 << 15))
#define PLATFORM_SP2_ID             (2 | (1 << 15))
#define PLATFORM_SP3_ID             (3 | (1 << 15))

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1)
#define PLATFORM_VM1_ID              1
#define PLATFORM_VM2_ID              2
#define PLATFORM_VM3_ID              3
#endif

/*
 * CPU info
 */
#define PLATFORM_NO_OF_CPUS 4
#endif /* _PAL_CONFIG_H_ */
