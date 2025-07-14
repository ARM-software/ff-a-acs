/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_COMMON_VAL_INTF_H_
#define _PAL_COMMON_VAL_INTF_H_

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include "pal_misc_asm.h"
#include "pal_config_def.h"

#define LOG_BUFFER_SIZE 8192
#define static_assert _Static_assert

/**
 *   @brief    - This function parses the input string and writes bytes into UART TX FIFO
 *   @param    - c       : Input Character
 *   @return   - SUCCESS/FAILURE
**/

uint32_t pal_print_driver(uint8_t c);

/**
 *   @brief    - Reads from given non-volatile address.
 *   @param    - offset  : Offset into nvmem
 *   @param    - buffer  : Pointer to destination address
 *   @param    - size    : Number of bytes
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_nvm_read(uint32_t offset, void *buffer, size_t size);

/**
 *   @brief    - Writes into given non-volatile address.
 *   @param    - offset  : Offset into nvmem
 *   @param    - buffer  : Pointer to source address
 *   @param    - size    : Number of bytes
 *   @return   - SUCCESS/FAILURE
**/
uint32_t pal_nvm_write(uint32_t offset, void *buffer, size_t size);

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


#if (defined(SP1_COMPILE) || defined(SP2_COMPILE) || defined(VM2_COMPILE) ||\
     defined(SP3_COMPILE) || defined(SP4_COMPILE) || defined(VM3_COMPILE))
/* Use hyp log system call for sp2, sp3, sp4, vm2 and vm3 */
#define pal_uart_putc(x) pal_uart_putc_hypcall((char)x)
#else
/* Use platform uart for vm1 & sp1 */
#define pal_uart_putc(x) pal_print_driver(x)
#endif

#endif /* _PAL_COMMON_VAL_INTF_H_ */
