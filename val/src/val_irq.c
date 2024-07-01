/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_interfaces.h"
#ifndef TARGET_LINUX
#include <pal_spm_helpers.h>
#endif

void val_irq_setup(void)
{
    pal_irq_setup();
}

void val_irq_enable(uint32_t irq_num, uint8_t irq_priority)
{
    pal_irq_enable(irq_num, irq_priority);
}

void val_irq_disable(uint32_t irq_num)
{
    pal_irq_disable(irq_num);
}

int val_irq_register_handler(uint32_t num, void *irq_handler)
{
    return pal_irq_register_handler(num, irq_handler);
}

int val_irq_unregister_handler(uint32_t irq_num)
{
    return pal_irq_unregister_handler(irq_num);
}

void val_secure_intr_enable(uint32_t int_id, enum interrupt_pin pin)
{
    pal_secure_intr_enable(int_id, pin);
}

void val_secure_intr_disable(uint32_t int_id, enum interrupt_pin pin)
{
    pal_secure_intr_disable(int_id, pin);
}

uint64_t val_sleep(uint32_t ms)
{
#ifndef TARGET_LINUX
    return pal_sleep(ms);
#else
    return 0;
#endif
}

uint32_t val_interrupt_get(void)
{
#ifndef TARGET_LINUX
    return spm_interrupt_get();
#else
    return 0;
#endif
}

void val_sp_sleep(uint64_t ms)
{
#ifndef TARGET_LINUX
    sp_sleep(ms);
#else
    return;
#endif
}

