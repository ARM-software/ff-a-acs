/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_INTERFACES_H_
#define _PAL_INTERFACES_H_

#include "pal.h"
#include "pal_config_def.h"
#include "pal_memory.h"
#include "pal_endpoint_info.h"
#include "pal_libc.h"
#include "pal_mmio.h"
#include "pal_misc_asm.h"
#include "pal_irq.h"
#include "pal_shemaphore.h"
#ifndef TARGET_LINUX
#include "pal_arch_helpers.h"
#endif

/**
 *   @brief    - This function prints the given string and data onto the uart
 *   @param    - str      : Input String
 *   @param    - data1    : Value for first format specifier
 *   @param    - data2    : Value for second format specifier
 *   @return   - SUCCESS(0)/FAILURE(Any positive number)
**/
uint32_t pal_printf(const char *msg, uint64_t data1, uint64_t data2);

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

/**
 * @brief       - Generic handler called upon reception of an IRQ.
 *              - This function acknowledges the interrupt, calls the user-defined handler
 *              - if one has been registered then marks the processing of the interrupt as
 *              - complete.
 * @param       - Void.
 * @return      - SUCCESS/FAILURE.
**/
int pal_irq_handler_dispatcher(void);

/**
 * @brief       - Enable interrupt #irq_num for the calling core.
 * @param       - irq_num:       irq number.
 * @param       - irq_priority:  irq priority value.
 * @return      - Void.
**/
void pal_irq_enable(int irq_num, uint8_t irq_priority);

/**
 * @brief       - Disable interrupt #irq_num for the calling core.
 * @param       - irq_num: irq number.
 * @return      - Void
**/
void pal_irq_disable(int irq_num);

/**
 * @brief       - Register an interrupt handler for a given interrupt number.
 *              - Will fail if there is already an interrupt handler registered for
 *              - the same interrupt.
 * @param       - irq_num:      irq number.
 * @param       - irq_handler:  irq handler.
 * @return      - Return 0 on success, a negative value otherwise.
**/
int pal_irq_register_handler(int irq_num, void *irq_handler);

/**
 * @brief       - Unregister an interrupt handler for a given interrupt number.
 *              - Will fail if there is no interrupt handler registered for that interrupt.
 * @param       - irq_num:  irq number.
 * @return      - Return 0 on success, a negative value otherwise.
**/
int pal_irq_unregister_handler(int irq_num);

/**
 * @brief       - Send an SGI to a given core.
 * @param       - sgi_id:    SGI interrupt number.
 * @param       - core_pos:  CPU core number.
 * @return      - Void.
**/
void pal_send_sgi(int sgi_id, unsigned int core_pos);

#endif /* _PAL_INTERFACES_H_ */
