/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_interfaces.h"

uint32_t val_twdog_enable(uint32_t ms)
{
    return pal_twdog_enable(ms);
}

uint32_t val_twdog_disable(void)
{
    return pal_twdog_disable();
}

void val_twdog_intr_enable(void)
{
    pal_twdog_intr_enable();
}

void val_twdog_intr_disable(void)
{
    pal_twdog_intr_disable();
}

void val_ns_wdog_enable(uint32_t ms)
{
    pal_ns_wdog_enable(ms);
}

void val_ns_wdog_disable(void)
{
    pal_ns_wdog_disable();
}

void val_ns_wdog_intr_enable(void)
{
    pal_ns_wdog_intr_enable();
}

void val_ns_wdog_intr_disable(void)
{
    pal_ns_wdog_intr_disable();
}
