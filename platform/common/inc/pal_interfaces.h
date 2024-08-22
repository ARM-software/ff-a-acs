/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_INTERFACES_H_
#define _PAL_INTERFACES_H_

#include "pal.h"
#include "pal_log.h"
#include "pal_config_def.h"
#include "pal_memory.h"
#include "pal_endpoint_info.h"
#include "pal_libc.h"
#include "pal_mmio.h"
#include "pal_misc_asm.h"
#include "pal_irq.h"
#include "pal_shemaphore.h"
#ifndef TARGET_LINUX
#include <pal_sp805_watchdog.h>
#include "pal_arch_helpers.h"
#endif

/**
 *   @brief    - This function prints the given string and data onto the uart
 *   @param    - str      : Input String
 *   @param    - ...      : ellipses for variadic args 
 *   @return   - SUCCESS((Any positive number for character written)/FAILURE(0)
**/
uint32_t pal_printf(print_verbosity_t verbosity, const char *msg, ...);

/**
 *   @brief    - Writes into given non-volatile address.
 *   @param    - offset  : Offset into nvmem
 *   @param    - buffer  : Pointer to source address
 *   @param    - size    : Number of bytes
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_nvm_write(uint32_t offset, void *buffer, size_t size);

/**
 *   @brief    - Reads from given non-volatile address.
 *   @param    - offset  : Offset into nvmem
 *   @param    - buffer  : Pointer to destination address
 *   @param    - size    : Number of bytes
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_nvm_read(uint32_t offset, void *buffer, size_t size);

/**
 *   @brief    - Initializes and enable the hardware watchdog timer
 *   @param    - void
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_watchdog_enable(void);

/**
 *   @brief    - Disables the hardware watchdog timer
 *   @param    - void
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_watchdog_disable(void);

/**
 *   @brief    - Initializes and enable physical system timer with interrupt
 *   @param    - us   : timeout (Microseconds)
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_ap_phy_refclk_en(uint32_t us);

/**
 *   @brief    - Disables physical system timer and interrupt
 *   @param    - int_mask   : mask interrupt
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_ap_phy_refclk_dis(bool int_mask);

/**
 *   @brief    - Initializes and enable virtual system timer with interrupt
 *   @param    - us   : timeout (Microseconds)
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_ap_virt_refclk_en(uint32_t us);

/**
 *   @brief    - Disables virtual system timer and interrupt
 *   @param    - int_mask   : mask interrupt
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_ap_virt_refclk_dis(bool int_mask);

/**
 *   @brief    - Initializes and enable the hardware watchdog timer
 *   @param    - ms   : Number of cycles(Milliseconds)
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_twdog_enable(uint32_t ms);

/**
 *   @brief    - Disables the hardware trusted watchdog timer
 *   @param    - void
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_twdog_disable(void);

/**
 *   @brief    - Returns the base address of endpoint device map region block.
 *                This data is used to map device regions into endpoint translation table.
 *   @param    - region_list: Populates the base address of device map region block
 *   @param    - no_of_mem_regions: Populates number of device regions assigned to endpoint
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_get_endpoint_device_map(void **region_list, size_t *no_of_mem_regions);
/**
 *   @brief    - Returns number of cpus in the system.
 *   @param    - void
 *   @return   - Number of cpus
**/
uint32_t pal_get_no_of_cpus(void);

/**
 *   @brief    - Convert mpid to logical cpu number
 *   @param    - mpid value
 *   @return   - Logical cpu number
**/
uint32_t pal_get_cpuid(uint64_t mpid);

/**
 *   @brief    - Return mpid value of given logical cpu index
 *   @param    - Logical cpu index
 *   @return   - mpid value
**/
uint64_t pal_get_mpid(uint32_t cpuid);

/**
 *   @brief    - Power up the given core
 *   @param    - mpid value of the core
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_power_on_cpu(uint64_t mpid);

/**
 *   @brief    - Power down the calling core.
 *   @param    - Void
 *   @return   - The call does not return when successful. Otherwise, it returns PAL_ERROR.
**/
uint32_t pal_power_off_cpu(void);

/**
 *   @brief    - Terminates the simulation at the end of all tests completion.
 *   @param    - void
 *   @return   - SUCCESS(0)/FAILURE
**/
uint32_t pal_terminate_simulation(void);

/**
 * @brief       - Returns the physical address of the input virtual address
 * @param       - va: Virtual address of the memory to be converted
 * @return      - Physical address(PA or IPA)
**/
void *pal_mem_virt_to_phys(void *va);
/**
 * @brief       - Configures SMMU upstream device and initiates DMA transfer from
 *              - source address to destination.
 * @param       - stream_id of the device.
 * @param       - Source address.
 * @param       - Destination address.
 * @param       - Size of the memory.
 * @param       - Device attribute secure/non-secure.
 * @return      - SUCCESS/FAILURE.
**/
uint32_t pal_smmu_device_configure(uint32_t stream_id, uint64_t source, uint64_t dest,
                                     uint64_t size, bool secure);

/**
 * @brief       - Initialise the irq vector table.
 * @param       - Void.
 * @return      - Void.
**/
void pal_irq_setup(void);


uint64_t pal_sleep(uint32_t ms);
void pal_twdog_intr_enable(void);
void pal_twdog_intr_disable(void);
void pal_ns_wdog_enable(uint32_t ms);
void pal_ns_wdog_disable(void);
void pal_ns_wdog_intr_enable(void);
void pal_ns_wdog_intr_disable(void);
void pal_secure_intr_enable(uint32_t int_id, enum interrupt_pin pin);
void pal_secure_intr_disable(uint32_t int_id, enum interrupt_pin pin);

#endif /* _PAL_INTERFACES_H_ */
