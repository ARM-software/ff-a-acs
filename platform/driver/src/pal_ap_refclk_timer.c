/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_ap_refclk_timer.h"
#include "pal_interfaces.h"

/* read helpers */
static inline uint32_t ap_refclk_read_cntfrq(void)
{
    return pal_mmio_read32((uint64_t) (AP_REFCLK_BASE + AP_REFCLK_CNTFRQ_OFF));
}

static inline uint64_t ap_refclk_read_cntpct(void)
{
    return pal_mmio_read32((uint64_t) (AP_REFCLK_BASE + AP_REFCLK_CNTPCT_OFF));
}

static inline uint64_t ap_refclk_read_cntvct(void)
{
    return pal_mmio_read32((uint64_t) (AP_REFCLK_BASE + AP_REFCLK_CNTVCT_0FF));
}

static inline uint32_t ap_refclk_read_cntp_ctl(void)
{
    return pal_mmio_read32((uint64_t) (AP_REFCLK_BASE + AP_REFCLK_CNTP_CTL_OFF));
}

static inline uint32_t ap_refclk_read_cntv_ctl(void)
{
    return pal_mmio_read32((uint64_t) (AP_REFCLK_BASE + AP_REFCLK_CNTV_CTL_OFF));
}

/* write helpers */
static inline void ap_refclk_write_cntp_cval(uint64_t val)
{
    pal_mmio_write64((uint64_t) (AP_REFCLK_BASE + AP_REFCLK_CNTP_CVAL_OFF), val);
}

static inline void ap_refclk_write_cntv_cval(uint64_t val)
{
    pal_mmio_write64((uint64_t) (AP_REFCLK_BASE + AP_REFCLK_CNTV_CVAL_OFF), val);
}

static inline void ap_refclk_write_cntp_ctl(uint32_t val)
{
    pal_mmio_write32((uint64_t) (AP_REFCLK_BASE + AP_REFCLK_CNTP_CTL_OFF), val);
}

static inline void ap_refclk_write_cntv_ctl(uint32_t val)
{
    pal_mmio_write32((uint64_t) (AP_REFCLK_BASE + AP_REFCLK_CNTV_CTL_OFF), val);
}

void driver_ap_refclk_p_disable(bool int_mask)
{
    uint32_t timer_ctrl_reg = 0;

    /* Disable timer */
    timer_ctrl_reg = ap_refclk_read_cntp_ctl();
    if(int_mask)
    {
        timer_ctrl_reg |= AP_REFCLK_CNT_CTL_IMASK;
        timer_ctrl_reg &= (uint32_t)(~AP_REFCLK_CNT_CTL_ENABLE);
    } else {
        timer_ctrl_reg &= (uint32_t)(~(AP_REFCLK_CNT_CTL_ENABLE | AP_REFCLK_CNT_CTL_IMASK));
    }

    ap_refclk_write_cntp_ctl(timer_ctrl_reg);
}

void driver_ap_refclk_v_disable(bool int_mask)
{
    uint32_t timer_ctrl_reg = 0;

    /* Disable timer */
    timer_ctrl_reg = ap_refclk_read_cntv_ctl();
    if(int_mask)
    {
        timer_ctrl_reg |= AP_REFCLK_CNT_CTL_IMASK;
        timer_ctrl_reg &= (uint32_t)(~AP_REFCLK_CNT_CTL_ENABLE);
    } else {
        timer_ctrl_reg &= (uint32_t)(~(AP_REFCLK_CNT_CTL_ENABLE | AP_REFCLK_CNT_CTL_IMASK));
    }

    ap_refclk_write_cntv_ctl(timer_ctrl_reg);
}

void driver_ap_refclk_p_set(uint32_t us_timeout, bool irq_enable)
{
    uint64_t cval = 0;
    uint32_t freq = 0;
    uint32_t timer_ctrl_reg = 0;
    bool int_mask = false;

    /* Disable timer */
    driver_ap_refclk_p_disable(int_mask);

    /* Program the timer */
    freq = (uint32_t)(ap_refclk_read_cntfrq() / 1000000);
    assert(freq != 0);
    cval = ap_refclk_read_cntpct();
    cval += (freq * us_timeout);
    ap_refclk_write_cntp_cval(cval);

    /* Enable the timer */
    timer_ctrl_reg = ap_refclk_read_cntp_ctl();

    if (!irq_enable)
    {
        timer_ctrl_reg |= (AP_REFCLK_CNT_CTL_IMASK);
    } else {
        timer_ctrl_reg &= (uint32_t)(~AP_REFCLK_CNT_CTL_IMASK);
    }

    timer_ctrl_reg |= AP_REFCLK_CNT_CTL_ENABLE;
    ap_refclk_write_cntp_ctl(timer_ctrl_reg);
}

void driver_ap_refclk_v_set(uint32_t us_timeout, bool irq_enable)
{
    uint64_t cval = 0;
    uint32_t freq = 0;
    uint32_t timer_ctrl_reg = 0;
    bool int_mask = false;

    /* Disable timer */
    driver_ap_refclk_v_disable(int_mask);

    /* Program the timer */
    freq = (uint32_t)(ap_refclk_read_cntfrq() / 1000000);
    assert(freq != 0);
    cval = ap_refclk_read_cntvct();
    cval += (freq * us_timeout);
    ap_refclk_write_cntv_cval(cval);

    /* Enable the timer */
    timer_ctrl_reg = ap_refclk_read_cntv_ctl();

    if (!irq_enable)
    {
        timer_ctrl_reg |= (AP_REFCLK_CNT_CTL_IMASK);
    } else {
        timer_ctrl_reg &= (uint32_t)(~AP_REFCLK_CNT_CTL_IMASK);
    }

    timer_ctrl_reg |= AP_REFCLK_CNT_CTL_ENABLE;
    ap_refclk_write_cntv_ctl(timer_ctrl_reg);
}


