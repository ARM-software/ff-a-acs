/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <pal_arch_helpers.h>
#include <pal_arm_gic.h>
#include <pal_gic_common.h>
#include <pal_gic_v3.h>
#include <pal_interfaces.h>

/* Global variables to store the GIC base addresses */
static uintptr_t gicr_base_addr;
static uintptr_t gicd_base_addr;

#ifdef __aarch64__
#define MPIDR_AFFLVL3_MASK    ((unsigned long long)MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT)
#define gic_typer_affinity_from_mpidr(mpidr)    \
    (((mpidr) & (~MPIDR_AFFLVL3_MASK)) | (((mpidr) & MPIDR_AFFLVL3_MASK) >> 8))
#else
#define gic_typer_affinity_from_mpidr(mpidr)    \
    ((mpidr) & ((MPIDR_AFFLVL_MASK << MPIDR_AFF2_SHIFT) | MPID_MASK))
#endif

/* Array to store the per-cpu redistributor frame addresses */
static uintptr_t rdist_pcpu_base[PLATFORM_NO_OF_CPUS];

/*
 * Array to store the mpidr corresponding to each initialized per-CPU
 * redistributor interface.
 */
static unsigned long long mpidr_list[PLATFORM_NO_OF_CPUS] = {UINT64_MAX};

/******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 *****************************************************************************/
static void gicd_write_irouter(uintptr_t base,
                unsigned int interrupt_id,
                unsigned long long route)
{
    assert(interrupt_id >= MIN_SPI_ID);
    pal_mmio_write64(base + GICD_IROUTER + (interrupt_id << 3), route);
}

/******************************************************************************
 * GIC Re-distributor interface accessors for writing entire registers
 *****************************************************************************/
static void gicr_write_isenabler0(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICR_ISENABLER0, val);
}

static void gicr_write_icenabler0(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICR_ICENABLER0, val);
}

static void gicr_write_icpendr0(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICR_ICPENDR0, val);
}

/******************************************************************************
 * GIC Re-distributor interface accessors for reading entire registers
 *****************************************************************************/
static unsigned long long gicr_read_typer(uintptr_t base)
{
    return pal_mmio_read64(base + GICR_TYPER);
}

static unsigned int gicr_read_isenabler0(uintptr_t base)
{
    return pal_mmio_read32(base + GICR_ISENABLER0);
}

static unsigned int gicr_read_ispendr0(uintptr_t base)
{
    return pal_mmio_read32(base + GICR_ISPENDR0);
}

/******************************************************************************
 * GIC Re-distributor interface accessors for individual interrupt
 * manipulation
 *****************************************************************************/
static unsigned int gicr_get_isenabler0(uintptr_t base,
    int interrupt_id)
{
    unsigned int bit_num = interrupt_id & ((1 << ISENABLER_SHIFT) - 1);
    return (uint32_t)gicr_read_isenabler0(base) & (uint32_t)(1 << bit_num);
}

static void gicr_set_isenabler0(uintptr_t base, int interrupt_id)
{
    unsigned bit_num = interrupt_id & ((1 << ISENABLER_SHIFT) - 1);
    gicr_write_isenabler0(base, (uint32_t)(1 << bit_num));
}

static void gicr_set_icenabler0(uintptr_t base, int interrupt_id)
{
    unsigned bit_num = interrupt_id & ((1 << ISENABLER_SHIFT) - 1);
    gicr_write_icenabler0(base, (uint32_t)(1 << bit_num));
}

static void gicr_set_icpendr0(uintptr_t base, int interrupt_id)
{
    unsigned bit_num = interrupt_id & ((1 << ICPENDR_SHIFT) - 1);
    gicr_write_icpendr0(base, (uint32_t)(1 << bit_num));
}

/******************************************************************************
 * GICv3 public driver API
 *****************************************************************************/
void gicv3_enable_cpuif(void)
{
    /* Assert that system register access is enabled */
    assert(IS_IN_EL2() ? (read_icc_sre_el2() & ICC_SRE_SRE_BIT) :
                (read_icc_sre_el1() & ICC_SRE_SRE_BIT));

    /* Enable Group1 non secure interrupts */
    write_icc_igrpen1_el1(read_icc_igrpen1_el1() | IGRPEN1_EL1_ENABLE_BIT);
    isb();
}

void gicv3_setup_cpuif(void)
{
    /* Set the priority mask register to allow all interrupts to trickle in */
    write_icc_pmr_el1(GIC_PRI_MASK);
    isb();
    gicv3_enable_cpuif();
}

void gicv3_disable_cpuif(void)
{
    /* Disable Group1 non secure interrupts */
    write_icc_igrpen1_el1((uint32_t)read_icc_igrpen1_el1() &
            (uint32_t)~IGRPEN1_EL1_ENABLE_BIT);
    isb();
}

unsigned int gicv3_get_ipriorityr(int interrupt_id)
{
    unsigned int core_pos;
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    if (interrupt_id < MIN_SPI_ID)
    {
        core_pos = pal_get_cpuid(read_mpidr_el1());
        assert(rdist_pcpu_base[core_pos]);
        return pal_mmio_read8(rdist_pcpu_base[core_pos] + GICR_IPRIORITYR
                + (uint32_t)interrupt_id);
    }
    else
    {
        return pal_mmio_read8(gicd_base_addr +
            GICD_IPRIORITYR + (uint32_t)interrupt_id);
    }
}

void gicv3_set_ipriorityr(int interrupt_id,
                unsigned int priority)
{
    unsigned int core_pos;
    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    if (interrupt_id < MIN_SPI_ID)
    {
        core_pos = pal_get_cpuid(read_mpidr_el1());
        assert(rdist_pcpu_base[core_pos]);
        pal_mmio_write8(rdist_pcpu_base[core_pos] + GICR_IPRIORITYR
                + (uint32_t)interrupt_id, priority & GIC_PRI_MASK);
    }
    else
    {
        pal_mmio_write8(gicd_base_addr + GICD_IPRIORITYR + (uint32_t)interrupt_id,
                    priority & GIC_PRI_MASK);
    }
}

void gicv3_send_sgi(int sgi_id, unsigned int core_pos)
{
    unsigned long long aff0, aff1, aff2;
    unsigned long long sgir, target_list;

    assert(IS_SGI(sgi_id));
    assert(core_pos < PLATFORM_NO_OF_CPUS);

    assert(mpidr_list[core_pos] != UINT64_MAX);

    /* Extract the affinity information */
    aff0 = MPIDR_AFF_ID(mpidr_list[core_pos], 0);
    aff1 = MPIDR_AFF_ID(mpidr_list[core_pos], 1);
    aff2 = MPIDR_AFF_ID(mpidr_list[core_pos], 2);
#ifdef __aarch64__
    unsigned long long aff3;
    aff3 = MPIDR_AFF_ID(mpidr_list[core_pos], 3);
#endif

    /* Construct the SGI target list using Affinity 0 */
    assert(aff0 < SGI_TARGET_MAX_AFF0);
    target_list = 1ULL << aff0;

    /* Construct the SGI target affinity */
    sgir =
#ifdef __aarch64__
        ((aff3 & SGI1R_AFF_MASK) << SGI1R_AFF3_SHIFT) |
#endif
        ((aff2 & SGI1R_AFF_MASK) << SGI1R_AFF2_SHIFT) |
        ((aff1 & SGI1R_AFF_MASK) << SGI1R_AFF1_SHIFT) |
        ((target_list & SGI1R_TARGET_LIST_MASK)
                << SGI1R_TARGET_LIST_SHIFT);

    /* Combine SGI target affinity with the SGI ID */
    sgir |= (((uint64_t)sgi_id & SGI1R_INTID_MASK) << SGI1R_INTID_SHIFT);
#ifdef __aarch64__
    write_icc_sgi1r(sgir);
#else
    write64_icc_sgi1r(sgir);
#endif
    isb();
}

void gicv3_set_intr_route(int interrupt_id,
        unsigned int core_pos)
{
    unsigned long long route_affinity;

    assert(gicd_base_addr);
    assert(core_pos < PLATFORM_NO_OF_CPUS);
    assert(mpidr_list[core_pos] != UINT64_MAX);

    /* Routing information can be set only for SPIs */
    assert(IS_SPI(interrupt_id));
    route_affinity = mpidr_list[core_pos];

    gicd_write_irouter(gicd_base_addr, (uint32_t)interrupt_id, route_affinity);
}

unsigned int gicv3_get_isenabler(int interrupt_id)
{
    unsigned int core_pos;

    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    if (interrupt_id < MIN_SPI_ID)
    {
        core_pos = pal_get_cpuid(read_mpidr_el1());
        assert(rdist_pcpu_base[core_pos]);
        return gicr_get_isenabler0(rdist_pcpu_base[core_pos], interrupt_id);
    } else
        return gicd_get_isenabler(gicd_base_addr, (uint32_t)interrupt_id);
}

void gicv3_set_isenabler(int interrupt_id)
{
    unsigned int core_pos;

    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    if (interrupt_id < MIN_SPI_ID)
    {
        core_pos = pal_get_cpuid(read_mpidr_el1());
        assert(rdist_pcpu_base[core_pos]);
        gicr_set_isenabler0(rdist_pcpu_base[core_pos], interrupt_id);
    } else
        gicd_set_isenabler(gicd_base_addr, (uint32_t)interrupt_id);
}

void gicv3_set_icenabler(int interrupt_id)
{
    unsigned int core_pos;

    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    if (interrupt_id < MIN_SPI_ID)
    {
        core_pos = pal_get_cpuid(read_mpidr_el1());
        assert(rdist_pcpu_base[core_pos]);
        gicr_set_icenabler0(rdist_pcpu_base[core_pos], interrupt_id);
    } else
        gicd_set_icenabler(gicd_base_addr, (uint32_t)interrupt_id);
}

unsigned int gicv3_get_ispendr(int interrupt_id)
{
    unsigned int ispendr;
    unsigned int bit_pos, core_pos;

    assert(gicd_base_addr);
    assert(IS_VALID_INTR_ID(interrupt_id));

    if (interrupt_id < MIN_SPI_ID)
    {
        core_pos = pal_get_cpuid(read_mpidr_el1());
        assert(rdist_pcpu_base[core_pos]);
        ispendr = gicr_read_ispendr0(rdist_pcpu_base[core_pos]);
    } else
        ispendr = gicd_read_ispendr(gicd_base_addr, interrupt_id);

    bit_pos = (uint32_t)interrupt_id % (1 << ISPENDR_SHIFT);
    return !!(ispendr & (1U << bit_pos));
}

void gicv3_set_icpendr(unsigned int interrupt_id)
{
    unsigned int core_pos;

    assert(gicd_base_addr);
    assert(IS_SPI(interrupt_id) || IS_PPI(interrupt_id));

    if (interrupt_id < MIN_SPI_ID)
    {
        core_pos = pal_get_cpuid(read_mpidr_el1());
        assert(rdist_pcpu_base[core_pos]);
        gicr_set_icpendr0(rdist_pcpu_base[core_pos], (int)interrupt_id);

    } else
        gicd_set_icpendr(gicd_base_addr, interrupt_id);
}

void gicv3_probe_redistif_addr(void)
{
    unsigned long long typer_val;
    uintptr_t rdistif_base;
    unsigned long long affinity;
    unsigned int core_pos = pal_get_cpuid(read_mpidr_el1());

    assert(gicr_base_addr);

    /*
     * Return if the re-distributor base address is already populated
     * for this core.
     */
    if (rdist_pcpu_base[core_pos])
        return;

    /* Iterate over the GICR frames and find the matching frame*/
    rdistif_base = gicr_base_addr;
    affinity = gic_typer_affinity_from_mpidr(read_mpidr_el1() & MPIDR_AFFINITY_MASK);
    do {
        typer_val = gicr_read_typer(rdistif_base);
        if (affinity == ((typer_val >> TYPER_AFF_VAL_SHIFT) & TYPER_AFF_VAL_MASK)) {
            rdist_pcpu_base[core_pos] = rdistif_base;
            mpidr_list[core_pos] = read_mpidr_el1() & MPIDR_AFFINITY_MASK;
            return;
        }
        rdistif_base += (1 << GICR_PCPUBASE_SHIFT);
    } while (!(typer_val & TYPER_LAST_BIT));

    pal_printf("Re-distributor address not found for core %d\n", core_pos, 0);
    assert(0);
}

void gicv3_setup_distif(void)
{
    unsigned int gicd_ctlr;

    assert(gicd_base_addr);

    /* Check for system register support */
#ifdef __aarch64__
    assert(read_id_aa64pfr0_el1() &
            (ID_AA64PFR0_GIC_MASK << ID_AA64PFR0_GIC_SHIFT));
#else
    assert(read_id_pfr1() & (ID_PFR1_GIC_MASK << ID_PFR1_GIC_SHIFT));
#endif

    /* Assert that system register access is enabled */
    assert(is_sre_enabled());

    /* Enable the forwarding of interrupts to CPU interface */
    gicd_ctlr = gicd_read_ctlr(gicd_base_addr);

    /* Assert ARE_NS bit in GICD */
    assert(gicd_ctlr & (GICD_CTLR_ARE_NS_MASK << GICD_CTLR_ARE_NS_SHIFT));

    gicd_ctlr |= GICD_CTLR_ENABLE_GRP1A;
    gicd_write_ctlr(gicd_base_addr, gicd_ctlr);
}

void gicv3_init(uintptr_t gicr_base, uintptr_t gicd_base)
{
    assert(gicr_base);
    assert(gicd_base);

    gicr_base_addr = gicr_base;
    gicd_base_addr = gicd_base;
}
