/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_interfaces.h"

void val_irq_setup(void)
{
    pal_irq_setup();
}

int val_irq_handler_dispatcher(void)
{
    return pal_irq_handler_dispatcher();
}

void val_irq_enable(int irq_num, uint8_t irq_priority)
{
    pal_irq_enable(irq_num, irq_priority);
}

void val_irq_disable(int irq_num)
{
    pal_irq_disable(irq_num);
}

int val_irq_register_handler(int num, void *irq_handler)
{
    return pal_irq_register_handler(num, irq_handler);
}

int val_irq_unregister_handler(int irq_num)
{
    return pal_irq_unregister_handler(irq_num);
}

