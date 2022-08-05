/*
 * Copyright (c) 2021-2022, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <pal_arch.h>
#include <pal_arch_helpers.h>
#include <pal_arm_gic.h>
#include <pal_gic_common.h>
#include <pal_gic_v2.h>
#include <pal_mmio.h>
#include <platform.h>
#include <pal_config_def.h>

/*
 * Data structure to store the GIC per CPU context before entering
 * system suspend. Only the GIC context of first 32 interrupts (SGIs and PPIs)
 * will be saved. The GIC SPI context needs to be restored by the respective
 * drivers. The GICC_PMR is not saved here as it will be reinitialized during
 * GIC restore.
 */
struct gicv2_pcpu_ctx {
    unsigned int gicc_ctlr;
    unsigned int gicd_isenabler0;
    unsigned int gicd_ipriorityr[NUM_PCPU_INTR >> IPRIORITYR_SHIFT];
    unsigned int gicd_icfgr;
};

static struct gicv2_pcpu_ctx pcpu_gic_ctx[PLATFORM_NO_OF_CPUS];

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
uint8_t gicd_read_itargetsr_byte(unsigned int base, unsigned int interrupt_id)
{
    return pal_mmio_read8(base + GICD_ITARGETSR + interrupt_id);
}

unsigned int gicd_read_itargetsr(unsigned int base, unsigned int interrupt_id)
{
    unsigned n = interrupt_id >> ITARGETSR_SHIFT;
    return pal_mmio_read32(base + GICD_ITARGETSR + (n << 2));
}

unsigned int gicd_read_cpendsgir(unsigned int base, unsigned int interrupt_id)
{
    unsigned n = interrupt_id >> CPENDSGIR_SHIFT;
    return pal_mmio_read32(base + GICD_CPENDSGIR + (n << 2));
}

unsigned int gicd_read_spendsgir(unsigned int base, unsigned int interrupt_id)
{
    unsigned n = interrupt_id >> SPENDSGIR_SHIFT;
    return pal_mmio_read32(base + GICD_SPENDSGIR + (n << 2));
}

/*******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 ******************************************************************************/
void gicd_write_itargetsr(unsigned int base,
                unsigned int interrupt_id, unsigned int val)
{
    unsigned n = interrupt_id >> ITARGETSR_SHIFT;
    pal_mmio_write32(base + GICD_ITARGETSR + (n << 2), val);
}

void gicd_write_itargetsr_byte(unsigned int base,
                unsigned int interrupt_id, unsigned int val)
{
    pal_mmio_write8(base + GICD_ITARGETSR + interrupt_id, (uint8_t)val);
}

void gicd_write_cpendsgir(unsigned int base,
                unsigned int interrupt_id, unsigned int val)
{
    unsigned n = interrupt_id >> CPENDSGIR_SHIFT;
    pal_mmio_write32(base + GICD_CPENDSGIR + (n << 2), val);
}

void gicd_write_spendsgir(unsigned int base,
                unsigned int interrupt_id, unsigned int val)
{
    unsigned n = interrupt_id >> SPENDSGIR_SHIFT;
    pal_mmio_write32(base + GICD_SPENDSGIR + (n << 2), val);
}

/*******************************************************************************
 * GIC Distributor interface accessors for individual interrupt manipulation
 ******************************************************************************/
void gicd_set_itargetsr(unsigned int base,
            unsigned int interrupt_id, unsigned int iface)
{
    pal_mmio_write8(base + GICD_ITARGETSR + interrupt_id, (uint8_t)(1U << iface));
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
    gicc_write_ctlr((unsigned int)gicc_base_addr, gicc_ctlr);
}

void gicv2_probe_gic_cpu_id(void)
{
    unsigned int gicd_itargets_val, core_pos;

    assert(gicd_base_addr);
    core_pos = platform_get_core_pos(read_mpidr_el1());
    gicd_itargets_val = gicd_read_itargetsr((unsigned int)gicd_base_addr, 0);

    assert(gicd_itargets_val);

    /* Convert the bit pos returned by read of ITARGETSR0 to GIC CPU ID */
    gic_cpu_id[core_pos] = (unsigned int)__builtin_ctz(gicd_itargets_val);
}

void gicv2_setup_cpuif(void)
{
    assert(gicc_base_addr);

    /* Set the priority mask register to allow all interrupts to trickle in */
    gicc_write_pmr((unsigned int)gicc_base_addr, GIC_PRI_MASK);
    gicv2_enable_cpuif();
}

void gicv2_disable_cpuif(void)
{
    unsigned int gicc_ctlr;

    assert(gicc_base_addr);

    /* Disable non-secure interrupts and disable their bypass */
    gicc_ctlr = gicc_read_ctlr((unsigned int)gicc_base_addr);
    gicc_ctlr &= ~GICC_CTLR_ENABLE;
    gicc_ctlr |= FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1;
    gicc_write_ctlr((unsigned int)gicc_base_addr, gicc_ctlr);
}

void gicv2_save_cpuif_context(void)
{
    unsigned int core_pos = platform_get_core_pos(read_mpidr_el1());

    assert(gicc_base_addr);
    pcpu_gic_ctx[core_pos].gicc_ctlr =
                gicc_read_ctlr((unsigned int)gicc_base_addr);
}

void gicv2_restore_cpuif_context(void)
{
    unsigned int core_pos = platform_get_core_pos(read_mpidr_el1());

    assert(gicc_base_addr);

    /* The GICC_PMR is never modified, hence we initialize this register */
    gicc_write_pmr((unsigned int)gicc_base_addr, GIC_PRI_MASK);

    gicc_write_ctlr((unsigned int)gicc_base_addr,
            pcpu_gic_ctx[core_pos].gicc_ctlr);
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

/* Save the per-cpu GICD ISENABLER, IPRIORITYR and ICFGR registers */
void gicv2_save_sgi_ppi_context(void)
{
    unsigned int i;
    unsigned int core_pos = platform_get_core_pos(read_mpidr_el1());

    assert(gicd_base_addr);
    pcpu_gic_ctx[core_pos].gicd_isenabler0 =
                gicd_read_isenabler(gicd_base_addr, 0);

    /* Read the ipriority registers, 4 at a time */
    for (i = 0; i < (NUM_PCPU_INTR >> IPRIORITYR_SHIFT); i++)
        pcpu_gic_ctx[core_pos].gicd_ipriorityr[i] =
            gicd_read_ipriorityr(gicd_base_addr, i << IPRIORITYR_SHIFT);

    pcpu_gic_ctx[core_pos].gicd_icfgr =
            gicd_read_icfgr(gicd_base_addr, MIN_PPI_ID);
}

/* Restore the per-cpu GICD ISENABLER, IPRIORITYR and ICFGR registers */
void gicv2_restore_sgi_ppi_context(void)
{
    unsigned int i;
    unsigned int core_pos = platform_get_core_pos(read_mpidr_el1());

    assert(gicd_base_addr);

    /* Write the ipriority registers, 4 at a time */
    for (i = 0; i < (NUM_PCPU_INTR >> IPRIORITYR_SHIFT); i++)
        gicd_write_ipriorityr(gicd_base_addr, i << IPRIORITYR_SHIFT,
            pcpu_gic_ctx[core_pos].gicd_ipriorityr[i]);

    gicd_write_icfgr(gicd_base_addr, MIN_PPI_ID,
            pcpu_gic_ctx[core_pos].gicd_icfgr);

    gicd_write_isenabler(gicd_base_addr, 0,
            pcpu_gic_ctx[core_pos].gicd_isenabler0);
}

unsigned int gicv2_gicd_get_ipriorityr(unsigned int interrupt_id)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    return gicd_get_ipriorityr(gicd_base_addr, interrupt_id);
}

void gicv2_gicd_set_ipriorityr(unsigned int interrupt_id,
                unsigned int priority)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    gicd_set_ipriorityr(gicd_base_addr, interrupt_id, priority);
}

void gicv2_send_sgi(unsigned int sgi_id, unsigned int core_pos)
{
    unsigned int sgir_val;

    assert(gicd_base_addr);
    assert(IS_SGI(sgi_id));

    sgir_val = sgi_id << GICD_SGIR_INTID_SHIFT;
    sgir_val |= (1U << core_pos_to_gic_id(core_pos)) << GICD_SGIR_CPUTL_SHIFT;

    gicd_write_sgir((unsigned int)gicd_base_addr, sgir_val);
}

void gicv2_set_itargetsr(unsigned int num, unsigned int core_pos)
{
    unsigned int gic_cpu_id;
    assert(gicd_base_addr);
    assert(IS_SPI(num));

    gic_cpu_id = core_pos_to_gic_id(core_pos);
    gicd_set_itargetsr((unsigned int)gicd_base_addr, num, gic_cpu_id);
}

uint8_t gicv2_read_itargetsr_value(unsigned int num)
{
    return gicd_read_itargetsr_byte((unsigned int)gicd_base_addr, num);
}

void gicv2_set_itargetsr_value(unsigned int num, unsigned int val)
{
    assert(gicd_base_addr);
    assert(IS_SPI(num));

    gicd_write_itargetsr_byte((unsigned int)gicd_base_addr, num, val);

    assert(gicv2_read_itargetsr_value(num) == val);
}

unsigned int gicv2_gicd_get_isenabler(unsigned int num)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(num));

    return gicd_get_isenabler(gicd_base_addr, num);
}

void gicv2_gicd_set_isenabler(unsigned int num)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(num));

    gicd_set_isenabler(gicd_base_addr, num);
}

void gicv2_gicd_set_icenabler(unsigned int num)
{
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(num));

    gicd_set_icenabler(gicd_base_addr, num);
}

unsigned int gicv2_gicc_read_iar(void)
{
    assert(gicc_base_addr);
    return gicc_read_iar((unsigned int)gicc_base_addr);
}

unsigned int gicv2_gicd_get_ispendr(unsigned int interrupt_id)
{
    unsigned int ispendr;
    unsigned int bit_pos;

    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    ispendr = gicd_read_ispendr(gicd_base_addr, interrupt_id);
    bit_pos = interrupt_id % (1 << ISPENDR_SHIFT);

    return !!(ispendr & (1U << bit_pos));
}

void gicv2_gicd_set_ispendr(unsigned int interrupt_id)
{
    assert(gicd_base_addr);
    assert(IS_PPI(interrupt_id) || IS_SPI(interrupt_id));
    gicd_set_ispendr(gicd_base_addr, interrupt_id);
}

void gicv2_gicd_set_icpendr(unsigned int interrupt_id)
{
    assert(gicd_base_addr);
    assert(IS_PPI(interrupt_id) || IS_SPI(interrupt_id));

    gicd_set_icpendr(gicd_base_addr, interrupt_id);
}

void gicv2_gicc_write_eoir(unsigned int val)
{
    assert(gicc_base_addr);

    gicc_write_eoir((unsigned int)gicc_base_addr, val);
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
