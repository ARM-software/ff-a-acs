/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <pal_arch_helpers.h>
#include <pal_arm_gic.h>
#include <pal_gic_common.h>
#include <pal_gic_v2.h>
#include <pal_interfaces.h>

static uintptr_t gicc_base_addr;
static uintptr_t gicd_base_addr;

static unsigned int gic_cpu_id[PLATFORM_NO_OF_CPUS] = {UINT32_MAX};

/* Helper function to convert core pos to gic id */
static unsigned int core_pos_to_gic_id(unsigned int core_pos)
{
    assert(gic_cpu_id[core_pos] != UINT32_MAX);
    return gic_cpu_id[core_pos];
}

/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/
static uint8_t gicd_read_itargetsr_byte(uintptr_t base, unsigned int interrupt_id)
{
    return pal_mmio_read8(base + GICD_ITARGETSR + interrupt_id);
}

static unsigned int gicd_read_itargetsr(uintptr_t base, unsigned int interrupt_id)
{
    unsigned n = interrupt_id >> ITARGETSR_SHIFT;
    return pal_mmio_read32(base + GICD_ITARGETSR + (n << 2));
}

static void gicd_write_itargetsr_byte(uintptr_t base,
                unsigned int interrupt_id, uint8_t val)
{
    pal_mmio_write8(base + GICD_ITARGETSR + interrupt_id, val);
}

/*******************************************************************************
 * GIC Distributor interface accessors for individual interrupt manipulation
 ******************************************************************************/
static void gicd_set_itargetsr(uintptr_t base,
            unsigned int interrupt_id, uint8_t iface)
{
    pal_mmio_write8(base + GICD_ITARGETSR + interrupt_id, (uint8_t)(1 << iface));
}

/******************************************************************************
 * GICv2 public driver API
 *****************************************************************************/

void gicv2_enable_cpuif(void)
{
    unsigned int gicc_ctlr;

    assert(gicc_base_addr);

    /* Enable the GICC and disable bypass */
    gicc_ctlr = GICC_CTLR_ENABLE | FIQ_BYP_DIS_GRP1
                     | IRQ_BYP_DIS_GRP1;
    gicc_write_ctlr(gicc_base_addr, gicc_ctlr);
}

void gicv2_probe_gic_cpu_id(void)
{
    uint32_t gicd_itargets_val, core_pos;

    assert(gicd_base_addr);
    core_pos = pal_get_cpuid(read_mpidr_el1());
    gicd_itargets_val = gicd_read_itargetsr(gicd_base_addr, 0);

    assert(gicd_itargets_val);

    /* TODO */
    /* Convert the bit pos returned by read of ITARGETSR0 to GIC CPU ID */
    //gic_cpu_id[core_pos] = __builtin_ctz((int)gicd_itargets_val);
    gic_cpu_id[core_pos] = 1;
}

void gicv2_setup_cpuif(void)
{
    assert(gicc_base_addr);

    /* Set the priority mask register to allow all interrupts to trickle in */
    gicc_write_pmr(gicc_base_addr, GIC_PRI_MASK);
    gicv2_enable_cpuif();
}

void gicv2_disable_cpuif(void)
{
    int gicc_ctlr;

    assert(gicc_base_addr);

    /* Disable non-secure interrupts and disable their bypass */
    gicc_ctlr = (int)gicc_read_ctlr(gicc_base_addr);
    gicc_ctlr = gicc_ctlr & ~GICC_CTLR_ENABLE;
    gicc_ctlr |= FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;
    gicc_write_ctlr(gicc_base_addr, (uint32_t)gicc_ctlr);
}

void gicv2_setup_distif(void)
{
    unsigned int gicd_ctlr;

    assert(gicd_base_addr);

    /* Enable the forwarding of interrupts to CPU interface */
    gicd_ctlr = gicd_read_ctlr(gicd_base_addr);
    gicd_ctlr |= GICD_CTLR_ENABLE;
    gicd_write_ctlr(gicd_base_addr, gicd_ctlr);
}

unsigned int gicv2_gicd_get_ipriorityr(int interrupt_id)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    return gicd_get_ipriorityr(gicd_base_addr, interrupt_id);
}

void gicv2_gicd_set_ipriorityr(int interrupt_id,
                unsigned int priority)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    gicd_set_ipriorityr(gicd_base_addr, interrupt_id, priority);
}

void gicv2_send_sgi(int sgi_id, unsigned int core_pos)
{
    unsigned int sgir_val;

    assert(gicd_base_addr);
    assert(IS_SGI(sgi_id));

    sgir_val = (uint32_t)sgi_id << GICD_SGIR_INTID_SHIFT;
    sgir_val |= (uint32_t)(1 << core_pos_to_gic_id(core_pos)) << GICD_SGIR_CPUTL_SHIFT;

    gicd_write_sgir(gicd_base_addr, sgir_val);
}

void gicv2_set_itargetsr(int num, unsigned int core_pos)
{
    uint8_t gic_cpu_id;
    assert(gicd_base_addr);
    assert(IS_SPI(num));

    gic_cpu_id = (uint8_t)core_pos_to_gic_id(core_pos);
    gicd_set_itargetsr(gicd_base_addr, (uint32_t)num, gic_cpu_id);
}

uint8_t gicv2_read_itargetsr_value(unsigned int num)
{
    return gicd_read_itargetsr_byte(gicd_base_addr, num);
}

void gicv2_set_itargetsr_value(int num, unsigned int val)
{
    assert(gicd_base_addr);
    assert(IS_SPI(num));

    gicd_write_itargetsr_byte(gicd_base_addr, (uint32_t)num, (uint8_t)val);

    assert(gicv2_read_itargetsr_value((uint32_t)num) == val);
}

unsigned int gicv2_gicd_get_isenabler(int num)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(num));

    return gicd_get_isenabler(gicd_base_addr, (uint32_t)num);
}

void gicv2_gicd_set_isenabler(int num)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(num));

    gicd_set_isenabler(gicd_base_addr, (uint32_t)num);
}

void gicv2_gicd_set_icenabler(int num)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(num));

    gicd_set_icenabler(gicd_base_addr, (uint32_t)num);
}

unsigned int gicv2_gicc_read_iar(void)
{
    assert(gicc_base_addr);
    return gicc_read_iar(gicc_base_addr);
}

unsigned int gicv2_gicd_get_ispendr(int interrupt_id)
{
    int ispendr;
    int bit_pos;

    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    ispendr = (int)gicd_read_ispendr(gicd_base_addr, interrupt_id);
    bit_pos = interrupt_id % (1 << ISPENDR_SHIFT);

    return !!(ispendr & (1 << bit_pos));
}

void gicv2_gicd_set_ispendr(int interrupt_id)
{
    assert(gicd_base_addr);
    assert(IS_PPI(interrupt_id) || IS_SPI(interrupt_id));
    gicd_set_ispendr(gicd_base_addr, (uint32_t)interrupt_id);
}

void gicv2_gicd_set_icpendr(int interrupt_id)
{
    assert(gicd_base_addr);
    assert(IS_PPI(interrupt_id) || IS_SPI(interrupt_id));

    gicd_set_icpendr(gicd_base_addr, (uint32_t)interrupt_id);
}

void gicv2_gicc_write_eoir(unsigned int val)
{
    assert(gicc_base_addr);

    gicc_write_eoir(gicc_base_addr, val);
}

void gicv2_init(uintptr_t gicc_base,
        uintptr_t gicd_base)
{
    assert(gicc_base);
    assert(gicd_base);

    /* Assert that this is a GICv2 system */
    assert(!is_gicv3_mode());
    gicc_base_addr = gicc_base;
    gicd_base_addr = gicd_base;
}
