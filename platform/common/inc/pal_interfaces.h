/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
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
#include "pal_arch_helpers.h"
#include "pal_common_val_intf.h"
#ifndef TARGET_LINUX
#include <pal_sp805_watchdog.h>
#endif

/**
 *   @brief    - This function prints the given string and data onto the uart
 *   @param    - msg      : Input String
 *   @param    - data1    : Value for first format specifier
 *   @param    - data2    : Value for second format specifier
 *   @return   - SUCCESS/FAILURE
**/
#if (TARGET_LINUX == 1) && defined(VM1_COMPILE)
uint32_t pal_printf(const char *msg, ...);
#else
uint32_t pal_printf(const char *msg, uint64_t data1, uint64_t data2);
#endif

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
 *   @brief  Retrieves the base address of the heap buffer.
 *   @return Pointer to the heap buffer.
 */
void *pal_get_heap_buffer(void);

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

#ifdef TARGET_LINUX
/**
 * @brief       - Forward SMC Call to Kernel FF-A Driver.
 * @param       - FFA args.
 * @return      - Void.
**/
void pal_linux_call_conduit(void *args);
#endif
/**
 *   @brief    - get current sp/vm id
 *   @param    - Void
 *   @return   - FFA ID
**/
uint32_t pal_get_current_ep_id(void);

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