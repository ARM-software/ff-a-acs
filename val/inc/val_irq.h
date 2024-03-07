/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_IRQ_H_
#define _VAL_IRQ_H_

#include "val.h"

void val_irq_setup(void);
int val_irq_handler_dispatcher(void);
void val_irq_enable(uint32_t irq_num, uint8_t irq_priority);
void val_irq_disable(uint32_t irq_num);
int val_irq_register_handler(uint32_t num, void *irq_handler);
int val_irq_unregister_handler(uint32_t irq_num);
uint64_t val_sleep(uint32_t ms);
void val_secure_intr_enable(uint32_t int_id, enum interrupt_pin pin);
void val_secure_intr_disable(uint32_t int_id, enum interrupt_pin pin);
uint32_t val_interrupt_get(void);

#endif /* _VAL_IRQ_H_ */
