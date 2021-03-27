/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __GIC_V2_H__
#define __GIC_V2_H__

#include <pal_interfaces.h>

/***************************************************************************
 * Defines and prototypes specific to GIC v2.
 **************************************************************************/

/* GICD_CTLR bit definitions */
#define GICD_CTLR_ENABLE    (1 << 0)

/* Distributor interface register offsets */
#define GICD_ITARGETSR      0x800
#define GICD_SGIR           0xF00
#define GICD_CPENDSGIR      0xF10
#define GICD_SPENDSGIR      0xF20

/* GIC Distributor register shifts */
#define ITARGETSR_SHIFT        2
#define CPENDSGIR_SHIFT        2
#define SPENDSGIR_SHIFT        CPENDSGIR_SHIFT

/* GICD_SGIR bit shifts */
#define GICD_SGIR_INTID_SHIFT        0
#define GICD_SGIR_CPUTL_SHIFT        16

/* Physical CPU Interface register offsets */
#define GICC_CTLR       0x0
#define GICC_PMR        0x4
#define GICC_BPR        0x8
#define GICC_IAR        0xC
#define GICC_EOIR       0x10
#define GICC_RPR        0x14
#define GICC_HPPIR      0x18
#define GICC_AHPPIR     0x28
#define GICC_IIDR       0xFC
#define GICC_DIR        0x1000
#define GICC_PRIODROP        GICC_EOIR

/* GICC_IIDR bit masks and shifts */
#define GICC_IIDR_PID_SHIFT    20
#define GICC_IIDR_ARCH_SHIFT    16
#define GICC_IIDR_REV_SHIFT    12
#define GICC_IIDR_IMP_SHIFT    0

#define GICC_IIDR_PID_MASK    0xfff
#define GICC_IIDR_ARCH_MASK    0xf
#define GICC_IIDR_REV_MASK    0xf
#define GICC_IIDR_IMP_MASK    0xfff

/* HYP view virtual CPU Interface register offsets */
#define GICH_CTL        0x0
#define GICH_VTR        0x4
#define GICH_ELRSR0     0x30
#define GICH_ELRSR1     0x34
#define GICH_APR0       0xF0
#define GICH_LR_BASE    0x100

/* Virtual CPU Interface register offsets */
#define GICV_CTL            0x0
#define GICV_PRIMASK        0x4
#define GICV_BP             0x8
#define GICV_INTACK         0xC
#define GICV_EOI            0x10
#define GICV_RUNNINGPRI     0x14
#define GICV_HIGHESTPEND    0x18
#define GICV_DEACTIVATE     0x1000

/* GICC_IAR bit masks and shifts */
#define GICC_IAR_INTID_SHIFT    0
#define GICC_IAR_CPUID_SHIFT    10

#define GICC_IAR_INTID_MASK    0x3ff
#define GICC_IAR_CPUID_MASK    0x7

#define get_gicc_iar_intid(val)    (((val) >> GICC_IAR_INTID_SHIFT) \
                    & GICC_IAR_INTID_MASK)
#define get_gicc_iar_cpuid(val)    (((val) >> GICC_IAR_CPUID_SHIFT) \
                    & GICC_IAR_CPUID_MASK)

/*
 * GICC_CTLR is banked to provide Secure and Non-secure copies and the register
 * bit assignments are different in the Secure and Non-secure copies.
 * These are the bit assignments for the Non-secure copy.
 */
#define GICC_CTLR_ENABLE    (1 << 0)
#define FIQ_BYP_DIS_GRP1    (1 << 5)
#define IRQ_BYP_DIS_GRP1    (1 << 6)
#define EOI_MODE_NS         (1 << 9)

#ifndef __ASSEMBLY__

/*******************************************************************************
 * Private Interfaces for internal use by the GICv2 driver
 ******************************************************************************/

/*******************************************************************************
 * GICv2 Distributor interface accessors for reading/writing entire registers
 ******************************************************************************/
static inline unsigned int gicd_read_sgir(uintptr_t base)
{
    return pal_mmio_read32(base + GICD_SGIR);
}

static inline void gicd_write_sgir(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICD_SGIR, val);
}

/*******************************************************************************
 * GICv2 CPU interface accessors for reading entire registers
 ******************************************************************************/

static inline unsigned int gicc_read_ctlr(uintptr_t base)
{
    return pal_mmio_read32(base + GICC_CTLR);
}

static inline unsigned int gicc_read_pmr(uintptr_t base)
{
    return pal_mmio_read32(base + GICC_PMR);
}

static inline unsigned int gicc_read_bpr(uintptr_t base)
{
    return pal_mmio_read32(base + GICC_BPR);
}

static inline unsigned int gicc_read_iar(uintptr_t base)
{
    return pal_mmio_read32(base + GICC_IAR);
}

static inline unsigned int gicc_read_eoir(uintptr_t base)
{
    return pal_mmio_read32(base + GICC_EOIR);
}

static inline unsigned int gicc_read_hppir(uintptr_t base)
{
    return pal_mmio_read32(base + GICC_HPPIR);
}

static inline unsigned int gicc_read_ahppir(uintptr_t base)
{
    return pal_mmio_read32(base + GICC_AHPPIR);
}

static inline unsigned int gicc_read_dir(uintptr_t base)
{
    return pal_mmio_read32(base + GICC_DIR);
}

static inline unsigned int gicc_read_iidr(uintptr_t base)
{
    return pal_mmio_read32(base + GICC_IIDR);
}


/*******************************************************************************
 * GICv2 CPU interface accessors for writing entire registers
 ******************************************************************************/

static inline void gicc_write_ctlr(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICC_CTLR, val);
}

static inline void gicc_write_pmr(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICC_PMR, val);
}

static inline void gicc_write_bpr(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICC_BPR, val);
}


static inline void gicc_write_iar(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICC_IAR, val);
}

static inline void gicc_write_eoir(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICC_EOIR, val);
}

static inline void gicc_write_hppir(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICC_HPPIR, val);
}

static inline void gicc_write_dir(uintptr_t base, unsigned int val)
{
    pal_mmio_write32(base + GICC_DIR, val);
}

/******************************************************************************
 * GICv2 public driver API
 *****************************************************************************/

/*
 * Initialize the GICv2 driver. The base addresses of GIC CPU interface
 * `gicc_base` and the Distributor interface `gicd_base` must be provided
 * as arguments.
 */
void gicv2_init(uintptr_t gicc_base, uintptr_t gicd_base);

/*
 * Write the GICv2 EOIR register with `val` passed as argument. `val`
 * should be the raw value read from IAR register.
 */
void gicv2_gicc_write_eoir(unsigned int val);

/*
 * Set the bit corresponding to `interrupt_id` in the GICD ISPENDR register.
 */
void gicv2_gicd_set_ispendr(int interrupt_id);

/*
 * Set the bit corresponding to `interrupt_id` in the GICD ICPENDR register.
 */
void gicv2_gicd_set_icpendr(int interrupt_id);

/*
 * Get the bit corresponding to `interrupt_id` from the GICD ISPENDR register.
 */
unsigned int gicv2_gicd_get_ispendr(int interrupt_id);

/*
 * Read and return the value in GICC IAR register
 */
unsigned int gicv2_gicc_read_iar(void);

/*
 * Read and return the target core mask of interrupt ID `num`.
 */
uint8_t gicv2_read_itargetsr_value(unsigned int num);

/*
 * Set the bit corresponding to `num` in the GICD ICENABLER register.
 */
void gicv2_gicd_set_icenabler(int num);

/*
 * Get the bit corresponding to `num` in the GICD ISENABLER register.
 */
unsigned int gicv2_gicd_get_isenabler(int num);

/*
 * Set the bit corresponding to `num` in the GICD ISENABLER register.
 */
void gicv2_gicd_set_isenabler(int num);

/*
 * Set the target of interrupt ID `num` to core with index `core_pos`.
 */
void gicv2_set_itargetsr(int num, unsigned int core_pos);

/*
 * Set the target of interrupt ID `num` to the desired core mask.
 */
void gicv2_set_itargetsr_value(int num, unsigned int val);

/*
 * Send SGI with ID `sgi_id` to core with index `core_pos`.
 */
void gicv2_send_sgi(int sgi_id, unsigned int core_pos);

/*
 * Get the priority of the interrupt `interrupt_id`.
 */
unsigned int gicv2_gicd_get_ipriorityr(int interrupt_id);

/*
 * Set the priority of the interrupt `interrupt_id` to `priority`.
 */
void gicv2_gicd_set_ipriorityr(int interrupt_id, unsigned int priority);

/*
 * Setup the GIC Distributor interface.
 */
void gicv2_setup_distif(void);

/*
 * Disable the GIC CPU interface.
 */
void gicv2_disable_cpuif(void);

/*
 * Setup the GIC CPU interface.
 */
void gicv2_setup_cpuif(void);

/*
 * Enable the GIC CPU interface.
 */
void gicv2_enable_cpuif(void);

/*
 * Read the GICD ITARGETR0 to figure out the GIC ID for the current core.
 * This function is required to be invoked on successful boot of a core.
 * The GIC ID will be stored internally by the driver to convert core index
 * to GIC CPU ID when required.
 */
void gicv2_probe_gic_cpu_id(void);

#endif /*__ASSEMBLY__*/
#endif /* __GIC_V2_H__ */
