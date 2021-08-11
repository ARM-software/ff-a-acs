/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __GIC_V3_H__
#define __GIC_V3_H__

/***************************************************************************
 * Defines and prototypes specific to GIC v3.
 *************************************************************************/

/* GICD register offsets */
#define GICD_IROUTER        0x6000

/* GICD_CTLR bit definitions */
#define GICD_CTLR_ENABLE_GRP1A        (1 << 1)
#define GICD_CTLR_ARE_NS_SHIFT        4
#define GICD_CTLR_ARE_NS_MASK         0x1

/* GICR_TYPER bit definitions */
#define TYPER_AFF_VAL_SHIFT    32
#define TYPER_PROC_NUM_SHIFT   8
#define TYPER_LAST_SHIFT       4

#define TYPER_AFF_VAL_MASK    0xffffffff
#define TYPER_PROC_NUM_MASK   0xffff
#define TYPER_LAST_MASK       0x1

#define TYPER_LAST_BIT        (1 << TYPER_LAST_SHIFT)

/* GICD_IROUTER shifts and masks */
#define IROUTER_IRM_SHIFT   31
#define IROUTER_IRM_MASK    0x1

/*******************************************************************************
 * GICv3 Re-distributor interface registers & constants
 ******************************************************************************/
#define GICR_PCPUBASE_SHIFT    0x11
#define GICR_SGIBASE_OFFSET    (1 << 0x10)    /* 64 KB */
#define GICR_CTLR              0x0
#define GICR_TYPER             0x08
#define GICR_WAKER             0x14
#define GICR_IGROUPR0          (GICR_SGIBASE_OFFSET + 0x80)
#define GICR_ISENABLER0        (GICR_SGIBASE_OFFSET + 0x100)
#define GICR_ICENABLER0        (GICR_SGIBASE_OFFSET + 0x180)
#define GICR_ISPENDR0          (GICR_SGIBASE_OFFSET + 0x200)
#define GICR_ICPENDR0          (GICR_SGIBASE_OFFSET + 0x280)
#define GICR_IPRIORITYR        (GICR_SGIBASE_OFFSET + 0x400)
#define GICR_ICFGR0            (GICR_SGIBASE_OFFSET + 0xc00)
#define GICR_ICFGR1            (GICR_SGIBASE_OFFSET + 0xc04)
#define GICR_IGRPMODR0         (GICR_SGIBASE_OFFSET + 0xd00)

/*******************************************************************************
 * GICv3 CPU interface registers & constants
 ******************************************************************************/
/* ICC_SRE bit definitions*/
#define ICC_SRE_EN_BIT         (1 << 3)
#define ICC_SRE_DIB_BIT        (1 << 2)
#define ICC_SRE_DFB_BIT        (1 << 1)
#define ICC_SRE_SRE_BIT        (1 << 0)

/* ICC_IAR1_EL1 bit definitions */
#define IAR1_EL1_INTID_SHIFT        0
#define IAR1_EL1_INTID_MASK         0xffffff

/* ICC_SGI1R bit definitions */
#define SGI1R_TARGET_LIST_MASK        0xffff
#define SGI1R_TARGET_LIST_SHIFT       0x0
#define SGI1R_AFF_MASK                0xff
#define SGI1R_AFF1_SHIFT              16ULL
#define SGI1R_AFF2_SHIFT              32ULL
#ifdef __aarch64__
#define SGI1R_AFF3_SHIFT              48ULL
#endif
#define SGI1R_INTID_MASK        0xf
#define SGI1R_INTID_SHIFT       24
#define SGI1R_IRM_MASK          0x1
#define SGI1R_IRM_SHIFT         0x40

/* ICC_IGRPEN1_EL1 bit definitions */
#define IGRPEN1_EL1_ENABLE_SHIFT    0
#define IGRPEN1_EL1_ENABLE_BIT      (1 << IGRPEN1_EL1_ENABLE_SHIFT)

/* The highest affinity 0 that can be a SGI target*/
#define SGI_TARGET_MAX_AFF0        16

#ifndef ASSEMBLY

/*******************************************************************************
 * Helper GICv3 macros
 ******************************************************************************/
#define gicv3_acknowledge_interrupt()        read_icc_iar1_el1() &\
                            IAR1_EL1_INTID_MASK
#define gicv3_end_of_interrupt(id)        write_icc_eoir1_el1(id)

#define is_sre_enabled()    \
    (IS_IN_EL2() ? (read_icc_sre_el2() & ICC_SRE_SRE_BIT) :\
    (read_icc_sre_el1() & ICC_SRE_SRE_BIT))

/******************************************************************************
 * GICv3 public driver API
 *****************************************************************************/
 /*
  * Initialize the GICv3 driver. The base addresses of GIC Re-distributor
  * interface `gicr_base` and the Distributor interface `gicd_base` must
  * be provided as arguments.
  */
void gicv3_init(uintptr_t gicr_base, uintptr_t gicd_base);

/*
 * Setup the GIC Distributor interface.
 */
void gicv3_setup_distif(void);

/*
 * Probe the Re-distributor base corresponding to this core.
 * This function is required to be invoked on successful boot of a core.
 * The base address will be stored internally by the driver and will be
 * used when accessing the Re-distributor interface.
 */
void gicv3_probe_redistif_addr(void);

/*
 * Set the bit corresponding to `interrupt_id` in the ICPENDR register
 * at either Distributor or Re-distributor depending on the interrupt.
 */
void gicv3_set_icpendr(unsigned int interrupt_id);

/*
 * Get the bit corresponding to `interrupt_id` in the ISPENDR register
 * at either Distributor or Re-distributor depending on the interrupt.
 */
unsigned int gicv3_get_ispendr(int interrupt_id);

/*
 * Set the bit corresponding to `interrupt_id` in the ICENABLER register
 * at either Distributor or Re-distributor depending on the interrupt.
 */
void gicv3_set_icenabler(int interrupt_id);

/*
 * Get the bit corresponding to `interrupt_id` in the ISENABLER register
 * at either Distributor or Re-distributor depending on the interrupt.
 */
unsigned int gicv3_get_isenabler(int interrupt_id);

/*
 * Set the bit corresponding to `interrupt_id` in the ISENABLER register
 * at either Distributor or Re-distributor depending on the interrupt.
 */
void gicv3_set_isenabler(int interrupt_id);

/*
 * Set the `route` corresponding to `interrupt_id` in the IROUTER register
 * at Distributor.
 */
void gicv3_set_intr_route(int interrupt_id, unsigned int core_pos);

/*
 * Send SGI with ID `sgi_id` to core with index `core_pos`.
 */
void gicv3_send_sgi(int sgi_id, unsigned int core_pos);

/*
 * Get the priority of the interrupt `interrupt_id`.
 */
unsigned int gicv3_get_ipriorityr(int interrupt_id);

/*
 * Set the priority of the interrupt `interrupt_id` to `priority`.
 */
void gicv3_set_ipriorityr(int interrupt_id, unsigned int priority);

/*
 * Disable the GIC CPU interface.
 */
void gicv3_disable_cpuif(void);

/*
 * Setup the GIC CPU interface.
 */
void gicv3_setup_cpuif(void);

/*
 * Enable the GIC CPU interface.
 */
void gicv3_enable_cpuif(void);


#endif /*__ASSEMBLY__*/
#endif /* __GIC_V3_H__ */
