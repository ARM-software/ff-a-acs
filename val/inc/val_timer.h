/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_TIMER_H_
#define _VAL_TIMER_H_

#include "val.h"
#include "val_framework.h"

#define ARM_ARCH_TIMER_ENABLE           (1ULL << 0)
#define ARM_ARCH_TIMER_IMASK            (1ULL << 1)
#define ARM_ARCH_TIMER_ISTATUS          (1ULL << 2)

uint32_t val_sys_phy_timer_dis(bool int_mask);
uint32_t val_sys_phy_timer_en(uint32_t timeout);
uint32_t val_sys_virt_timer_dis(bool int_mask);
uint32_t val_sys_virt_timer_en(uint32_t timeout);
#endif /* _VAL_TIMER_H_ */
