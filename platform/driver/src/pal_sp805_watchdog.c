/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_sp805_watchdog.h"
#include "pal_interfaces.h"

static inline void sp805_write_wdog_load(unsigned long base, uint32_t value)
{
    pal_mmio_write32(base + SP805_WDOG_LOAD_OFF, value);
}

static inline void sp805_write_wdog_ctrl(unsigned long base, uint32_t value)
{
    /* Not setting reserved bits */
    pal_mmio_write32(base + SP805_WDOG_CTRL_OFF, value);
}

static inline void sp805_write_wdog_int_clr(unsigned long base, uint32_t value)
{
    pal_mmio_write32(base + SP805_WDOG_INT_CLR_OFF, value);
}

static inline void sp805_write_wdog_lock(unsigned long base, uint32_t value)
{
    pal_mmio_write32(base + SP805_WDOG_LOCK_OFF, value);
}

void driver_sp805_wdog_start(void)
{
    /* Unlock to access the watchdog registers */
    sp805_write_wdog_lock(SP805_WDOG_BASE, SP805_WDOG_UNLOCK_ACCESS);

    /* Write the number of cycles needed */
    sp805_write_wdog_load(SP805_WDOG_BASE, SP805_WDOG_LOAD_VALUE);

    /* Enable reset interrupt and watchdog interrupt on expiry */
    sp805_write_wdog_ctrl(SP805_WDOG_BASE,
            SP805_WDOG_CTRL_RESEN | SP805_WDOG_CTRL_INTEN);

    /* Lock registers so that they can't be accidently overwritten */
    sp805_write_wdog_lock(SP805_WDOG_BASE, 0x0);
}

void driver_sp805_wdog_stop(void)
{
    /* Unlock to access the watchdog registers */
    sp805_write_wdog_lock(SP805_WDOG_BASE, SP805_WDOG_UNLOCK_ACCESS);

    /* Clearing INTEN bit stops the counter */
    sp805_write_wdog_ctrl(SP805_WDOG_BASE, 0x00);

    /* Lock registers so that they can't be accidently overwritten */
    sp805_write_wdog_lock(SP805_WDOG_BASE, 0x0);
}

void driver_sp805_wdog_refresh(void)
{
    /* Unlock to access the watchdog registers */
    sp805_write_wdog_lock(SP805_WDOG_BASE, SP805_WDOG_UNLOCK_ACCESS);

    /*
     * Write of any value to WdogIntClr clears interrupt and reloads
     * the counter from the value in WdogLoad Register.
     */
    sp805_write_wdog_int_clr(SP805_WDOG_BASE, 1);

    /* Lock registers so that they can't be accidently overwritten */
    sp805_write_wdog_lock(SP805_WDOG_BASE, 0x0);
}
