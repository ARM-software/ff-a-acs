/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef ARCH_H
#define ARCH_H

/*
 * For those constants to be shared between C and other sources, apply a 'U',
 * 'UL', 'ULL', 'L' or 'LL' suffix to the argument only in C, to avoid
 * undefined or unintended behaviour.
 *
 * The GNU assembler and linker do not support these suffixes (it causes the
 * build process to fail) therefore the suffix is omitted when used in linker
 * scripts and assembler files.
*/
#if defined(__LINKER__) || defined(__ASSEMBLY__)
# define   U(_x)    (_x)
# define  UL(_x)    (_x)
# define ULL(_x)    (_x)
# define   L(_x)    (_x)
# define  LL(_x)    (_x)
#else
# define   U(_x)    (_x##U)
# define  UL(_x)    (_x##UL)
# define ULL(_x)    (_x##ULL)
# define   L(_x)    (_x##L)
# define  LL(_x)    (_x##LL)
#endif

#define FVP_MAX_CPUS_PER_CLUSTER 4
#define FVP_MAX_PE_PER_CPU 1

/*******************************************************************************
 * MPIDR macros
 ******************************************************************************/
#define MPIDR_MT_MASK        U(1 << 24)
#define MPIDR_CPU_MASK       MPIDR_AFFLVL_MASK
#define MPIDR_CLUSTER_MASK   (MPIDR_AFFLVL_MASK << MPIDR_AFFINITY_BITS)
#define MPIDR_AFFINITY_BITS  8
#define MPIDR_AFFLVL_MASK    ULL(0xff)
#define MPIDR_AFF0_SHIFT     0
#define MPIDR_AFF1_SHIFT     U(8)
#define MPIDR_AFF2_SHIFT     U(16)
#define MPIDR_AFF3_SHIFT     U(32)
#define MPIDR_AFF_SHIFT(_n)  MPIDR_AFF##_n##_SHIFT
#define MPIDR_AFFINITY_MASK  ULL(0xff00ffffff)
#define MPIDR_AFFLVL_SHIFT   U(3)
#define MPIDR_AFFLVL0        ULL(0x0)
#define MPIDR_AFFLVL1        ULL(0x1)
#define MPIDR_AFFLVL2        ULL(0x2)
#define MPIDR_AFFLVL3        ULL(0x3)
#define MPIDR_AFFLVL(_n)     MPIDR_AFFLVL##_n
#define MPIDR_AFFLVL0_VAL(mpidr) \
        (((mpidr) >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL1_VAL(mpidr) \
        (((mpidr) >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL2_VAL(mpidr) \
        (((mpidr) >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL3_VAL(mpidr) \
        (((mpidr) >> MPIDR_AFF3_SHIFT) & MPIDR_AFFLVL_MASK)
/*
 * The MPIDR_MAX_AFFLVL count starts from 0. Take care to
 * add one while using this macro to define array sizes.
 * TODO: Support only the first 3 affinity levels for now.
 */
#define MPIDR_MAX_AFFLVL    U(2)

#define MPID_MASK        (MPIDR_MT_MASK                 | \
                 (MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT) | \
                 (MPIDR_AFFLVL_MASK << MPIDR_AFF2_SHIFT) | \
                 (MPIDR_AFFLVL_MASK << MPIDR_AFF1_SHIFT) | \
                 (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT))

#define MPIDR_AFF_ID(mpid, n)                    \
    (((mpid) >> MPIDR_AFF_SHIFT(n)) & MPIDR_AFFLVL_MASK)

/*
 * An invalid MPID. This value can be used by functions that return an MPID to
 * indicate an error.
 */
#define INVALID_MPID        U(0xFFFFFFFF)

/*******************************************************************************
 * Definitions for CPU system register interface to GICv3
 ******************************************************************************/
#define ICC_IGRPEN1_EL1         S3_0_C12_C12_7
#define ICC_SGI1R               S3_0_C12_C11_5
#define ICC_SRE_EL1             S3_0_C12_C12_5
#define ICC_SRE_EL2             S3_4_C12_C9_5
#define ICC_SRE_EL3             S3_6_C12_C12_5
#define ICC_CTLR_EL1            S3_0_C12_C12_4
#define ICC_CTLR_EL3            S3_6_C12_C12_4
#define ICC_PMR_EL1             S3_0_C4_C6_0
#define ICC_RPR_EL1             S3_0_C12_C11_3
#define ICC_IGRPEN1_EL3         S3_6_c12_c12_7
#define ICC_IGRPEN0_EL1         S3_0_c12_c12_6
#define ICC_HPPIR0_EL1          S3_0_c12_c8_2
#define ICC_HPPIR1_EL1          S3_0_c12_c12_2
#define ICC_IAR0_EL1            S3_0_c12_c8_0
#define ICC_IAR1_EL1            S3_0_c12_c12_0
#define ICC_EOIR0_EL1           S3_0_c12_c8_1
#define ICC_EOIR1_EL1           S3_0_c12_c12_1
#define ICC_SGI0R_EL1           S3_0_c12_c11_7

/*******************************************************************************
 * Generic timer memory mapped registers & offsets
 ******************************************************************************/
#define CNTCR_OFF            U(0x000)
#define CNTFID_OFF           U(0x020)

#define CNTCR_EN            (U(1) << 0)
#define CNTCR_HDBG          (U(1) << 1)
#define CNTCR_FCREQ(x)      ((x) << 8)

/* ID_AA64PFR0_EL1 definitions */
#define ID_AA64PFR0_EL0_SHIFT    U(0)
#define ID_AA64PFR0_EL1_SHIFT    U(4)
#define ID_AA64PFR0_EL2_SHIFT    U(8)
#define ID_AA64PFR0_EL3_SHIFT    U(12)
#define ID_AA64PFR0_ELX_MASK     ULL(0xf)
#define EL_IMPL_NONE             ULL(0)
#define EL_IMPL_A64ONLY          ULL(1)
#define EL_IMPL_A64_A32          ULL(2)

#define ID_AA64PFR0_GIC_SHIFT    U(24)
#define ID_AA64PFR0_GIC_WIDTH    U(4)
#define ID_AA64PFR0_GIC_MASK     ULL(0xf)

/* CPSR/SPSR definitions */
#define DAIF_FIQ_BIT        (U(1) << 0)
#define DAIF_IRQ_BIT        (U(1) << 1)
#define DAIF_ABT_BIT        (U(1) << 2)
#define DAIF_DBG_BIT        (U(1) << 3)
#define SPSR_DAIF_SHIFT     U(6)
#define SPSR_DAIF_MASK      U(0xf)

#define MODE_RW_SHIFT        U(0x4)
#define MODE_RW_MASK         U(0x1)
#define MODE_RW_64           U(0x0)
#define MODE_RW_32           U(0x1)

#define MODE_EL_SHIFT        U(0x2)
#define MODE_EL_MASK         U(0x3)
#define MODE_EL3             U(0x3)
#define MODE_EL2             U(0x2)
#define MODE_EL1             U(0x1)
#define MODE_EL0             U(0x0)

#define GET_RW(mode)        (((mode) >> MODE_RW_SHIFT) & MODE_RW_MASK)
#define GET_EL(mode)        (((mode) >> MODE_EL_SHIFT) & MODE_EL_MASK)
#define GET_SP(mode)        (((mode) >> MODE_SP_SHIFT) & MODE_SP_MASK)
#define GET_M32(mode)        (((mode) >> MODE32_SHIFT) & MODE32_MASK)

#endif /* ARCH_H */
