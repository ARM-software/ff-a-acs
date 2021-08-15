/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <pal_arch_helpers.h>
#include <pal_gic_common.h>
#include <pal_gic_v2.h>
#include <pal_gic_v3.h>
#include <pal_arm_gic.h>
#include "pal_interfaces.h"

/* Record whether a GICv3 was detected on the system */
static unsigned int gicv3_detected;

void arm_gic_enable_interrupts_local(void)
{
    if (gicv3_detected)
        gicv3_enable_cpuif();
    else
        gicv2_enable_cpuif();
}

void arm_gic_setup_local(void)
{
    if (gicv3_detected)
    {
        gicv3_probe_redistif_addr();
        gicv3_setup_cpuif();
    }
    else
    {
        gicv2_probe_gic_cpu_id();
        gicv2_setup_cpuif();
    }
}

void arm_gic_disable_interrupts_local(void)
{
    if (gicv3_detected)
        gicv3_disable_cpuif();
    else
        gicv2_disable_cpuif();
}

void arm_gic_setup_global(void)
{
    if (gicv3_detected)
        gicv3_setup_distif();
    else
        gicv2_setup_distif();
}

unsigned int arm_gic_get_intr_priority(int num)
{
    if (gicv3_detected)
        return gicv3_get_ipriorityr(num);
    else
        return gicv2_gicd_get_ipriorityr(num);
}

void arm_gic_set_intr_priority(int num,
                unsigned int priority)
{
    if (gicv3_detected)
        gicv3_set_ipriorityr(num, priority);
    else
        gicv2_gicd_set_ipriorityr(num, priority);
}

void arm_gic_send_sgi(int sgi_id, unsigned int core_pos)
{
    if (gicv3_detected)
        gicv3_send_sgi(sgi_id, core_pos);
    else
        gicv2_send_sgi(sgi_id, core_pos);
}

void arm_gic_set_intr_target(int num, unsigned int core_pos)
{
    if (gicv3_detected)
        gicv3_set_intr_route(num, core_pos);
    else
        gicv2_set_itargetsr(num, core_pos);
}

unsigned int arm_gic_intr_enabled(int num)
{
    if (gicv3_detected)
        return gicv3_get_isenabler(num) != 0;
    else
        return gicv2_gicd_get_isenabler(num) != 0;
}

void arm_gic_intr_enable(int num)
{
    if (gicv3_detected)
        gicv3_set_isenabler(num);
    else
        gicv2_gicd_set_isenabler(num);
}

void arm_gic_intr_disable(int num)
{
    if (gicv3_detected)
        gicv3_set_icenabler(num);
    else
        gicv2_gicd_set_icenabler(num);
}

unsigned int arm_gic_intr_ack(unsigned int *raw_iar)
{
    assert(raw_iar);

    if (gicv3_detected)
    {
        *raw_iar = gicv3_acknowledge_interrupt();
        return *raw_iar;
    }
    else
    {
        *raw_iar = gicv2_gicc_read_iar();
        return get_gicc_iar_intid(*raw_iar);
    }
}

unsigned int arm_gic_is_intr_pending(unsigned int num)
{
    if (gicv3_detected)
        return gicv3_get_ispendr((int)num);
    else
        return gicv2_gicd_get_ispendr((int)num);
}

void arm_gic_intr_clear(unsigned int num)
{
    if (gicv3_detected)
        gicv3_set_icpendr(num);
    else
        gicv2_gicd_set_icpendr((int)num);
}

void arm_gic_end_of_intr(unsigned int raw_iar)
{
    if (gicv3_detected)
        gicv3_end_of_interrupt(raw_iar);
    else
        gicv2_gicc_write_eoir(raw_iar);
}

void arm_gic_init(uintptr_t gicc_base, uintptr_t gicd_base,
                  uintptr_t gicr_base)
{
    if (is_gicv3_mode())
    {
        gicv3_detected = 1;
        gicv3_init(gicr_base, gicd_base);
        pal_printf("GICv3 mode detected\n", 0, 0);
    }
    else
    {
        gicv2_init(gicc_base, gicd_base);
        pal_printf("GICv2 mode detected\n", 0, 0);
    }
}
