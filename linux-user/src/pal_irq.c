/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_interfaces.h"
#include "pal_irq.h"

void pal_irq_setup(void)
{
    return;
}

int pal_irq_handler_dispatcher(void)
{
    return 0;
}

void pal_irq_enable(unsigned int irq_num, uint8_t irq_priority)
{
    (void)irq_num;
    (void)irq_priority;
    return;
}

void pal_irq_disable(unsigned int irq_num)
{
    (void)irq_num;
    return;
}

int pal_irq_register_handler(unsigned int irq_num, handler_irq_t irq_handler)
{
    (void)irq_num;
    (void)irq_handler;
    return PAL_SUCCESS;
}

int pal_irq_unregister_handler(unsigned int irq_num)
{
    (void)irq_num;
    return PAL_SUCCESS;
}

void pal_send_sgi(unsigned int sgi_id, unsigned int core_pos)
{
    (void)sgi_id;
    (void)core_pos;
    return;
}
