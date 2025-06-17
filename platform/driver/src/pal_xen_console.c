/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_xen_console.h"

#include <xen/hypercall.h>
#include <xen/public/xen.h>


#define CONSOLEIO_write     0
#define __HYPERVISOR_console_io 18

char buffer[16];

void driver_xen_console_putc(char c)
{
    buffer[0] = c;
    buffer[1] = 0;

    HYPERVISOR_console_io(CONSOLEIO_write, 2, buffer);
}
