/*
 * Copyright (c) 2021-2022, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <pal_arch.h>
#include <pal_arch_helpers.h>
#include <pal_libc.h>
#include <pal_gic_v2.h>
#include <pal_arm_gic.h>

void arm_gic_enable_interrupts_local(void)
{
    gicv2_enable_cpuif();
}

void arm_gic_setup_local(void)
{
    gicv2_probe_gic_cpu_id();
    gicv2_setup_cpuif();
}

void arm_gic_disable_interrupts_local(void)
{
    gicv2_disable_cpuif();
}

void arm_gic_save_context_local(void)
{
    gicv2_save_cpuif_context();
}

void arm_gic_restore_context_local(void)
{
    gicv2_restore_cpuif_context();
}

void arm_gic_save_context_global(void)
{
    gicv2_save_sgi_ppi_context();
}

void arm_gic_restore_context_global(void)
{
    gicv2_setup_distif();
    gicv2_restore_sgi_ppi_context();
}

void arm_gic_setup_global(void)
{
    gicv2_setup_distif();
}

unsigned int arm_gic_get_intr_priority(unsigned int num)
{
    return gicv2_gicd_get_ipriorityr(num);
}

void arm_gic_set_intr_priority(unsigned int num,
                unsigned int priority)
{
    gicv2_gicd_set_ipriorityr(num, priority);
}

void arm_gic_send_sgi(unsigned int sgi_id, unsigned int core_pos)
{
    gicv2_send_sgi(sgi_id, core_pos);
}

void arm_gic_set_intr_target(unsigned int num, unsigned int core_pos)
{
    gicv2_set_itargetsr(num, core_pos);
}

unsigned int arm_gic_intr_enabled(unsigned int num)
{
    return gicv2_gicd_get_isenabler(num) != 0;
}

void arm_gic_intr_enable(unsigned int num)
{
    gicv2_gicd_set_isenabler(num);
}

void arm_gic_intr_disable(unsigned int num)
{
    gicv2_gicd_set_icenabler(num);
}

unsigned int arm_gic_intr_ack(unsigned int *raw_iar)
{
    assert(raw_iar);

    *raw_iar = gicv2_gicc_read_iar();
    return get_gicc_iar_intid(*raw_iar);
}

unsigned int arm_gic_is_intr_pending(unsigned int num)
{
    return gicv2_gicd_get_ispendr(num);
}

void arm_gic_intr_clear(unsigned int num)
{
    gicv2_gicd_set_icpendr(num);
}

void arm_gic_end_of_intr(unsigned int raw_iar)
{
    gicv2_gicc_write_eoir(raw_iar);
}

void arm_gic_init(uintptr_t gicc_base,
        uintptr_t gicd_base,
        uintptr_t gicr_base)
{
    gicv2_init(gicc_base, gicd_base);
    (void)gicr_base;
}

