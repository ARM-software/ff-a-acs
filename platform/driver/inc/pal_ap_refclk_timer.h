/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_AP_REFCLK_TIMER_H_
#define _PAL_AP_REFCLK_TIMER_H_

#include <pal_interfaces.h>

#define AP_REFCLK_BASE          PLATFORM_AP_REFCLK_CNTBASE1

/* AP_REFCLK register offset */
#define AP_REFCLK_CNTPCT_OFF       0x000
#define AP_REFCLK_CNTVCT_0FF       0x008
#define AP_REFCLK_CNTFRQ_OFF       0x010
#define AP_REFCLK_CNTVOFF_OFF      0x018
#define AP_REFCLK_CNTP_CVAL_OFF    0x020
#define AP_REFCLK_CNTP_TVAL_OFF    0x028
#define AP_REFCLK_CNTP_CTL_OFF     0x02c
#define AP_REFCLK_CNTV_CVAL_OFF    0x030
#define AP_REFCLK_CNTV_TVAL_OFF    0x038
#define AP_REFCLK_CNTV_CTL_OFF     0x03c

/* Register field definitions */
#define AP_REFCLK_CNT_CTL_ENABLE        (1 << 0)
#define AP_REFCLK_CNT_CTL_IMASK         (1 << 1)
#define AP_REFCLK_CNT_CTL_ISTATUS       (1 << 2)

void driver_ap_refclk_p_disable(bool int_mask);
void driver_ap_refclk_v_disable(bool int_mask);

void driver_ap_refclk_p_set(uint32_t us, bool int_enable);
void driver_ap_refclk_v_set(uint32_t us, bool int_enable);

#endif /* _PAL_AP_REFCLK_TIMER_H_ */

