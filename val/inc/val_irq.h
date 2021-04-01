/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_IRQ_H_
#define _VAL_IRQ_H_

#include "val.h"

void val_irq_setup(void);
int val_irq_handler_dispatcher(void);
void val_irq_enable(int irq_num, uint8_t irq_priority);
void val_irq_disable(int irq_num);
int val_irq_register_handler(int num, void *irq_handler);
int val_irq_unregister_handler(int irq_num);

#endif /* _VAL_IRQ_H_ */
