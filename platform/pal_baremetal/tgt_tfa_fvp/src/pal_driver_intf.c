/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"
#include "pal_pl011_uart.h"

#if defined(VM1_COMPILE) && defined(XEN_SUPPORT)
#include <pal_xen_pvconsole.h>
#endif

/**
    @brief    - This function parses the input string and writes bytes into UART TX FIFO
    @param    - c   : Input Character
    @return   - SUCCESS/FAILURE
**/

uint32_t pal_print_driver(uint8_t c)
{
#if defined(VM1_COMPILE) && defined(XEN_SUPPORT)
    driver_xen_pvconsole_putc(c);
#else
/* Use platform uart for vm1 & sp1 */
    driver_uart_pl011_putc(c);
#endif
    return PAL_SUCCESS;
}
