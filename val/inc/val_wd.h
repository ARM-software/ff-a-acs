/*
 *  * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *   *
 *    * SPDX-License-Identifier: BSD-3-Clause
 *     *
 *      */

#ifndef _VAL_WD_H_
#define _VAL_WD_H_

#include "val.h"

uint32_t val_twdog_enable(uint32_t ms);
uint32_t val_twdog_disable(void);
void val_twdog_intr_enable(void);
void val_twdog_intr_disable(void);
void val_ns_wdog_enable(uint32_t ms);
void val_ns_wdog_disable(void);
void val_ns_wdog_intr_enable(void);
void val_ns_wdog_intr_disable(void);

#endif /* _VAL_WD_H_ */
