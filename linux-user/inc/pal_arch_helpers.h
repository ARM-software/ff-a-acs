/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef ARCH_HELPERS_H
#define ARCH_HELPERS_H

#define MPIDR_MT_MASK        (1 << 24)
#define MPIDR_AFFLVL_MASK    0xffUL
#define MPIDR_AFF0_SHIFT     0
#define MPIDR_AFF1_SHIFT     (8)
#define MPIDR_AFF2_SHIFT     (16)
#define MPIDR_AFF3_SHIFT     32UL

/*
 * The MPIDR_MAX_AFFLVL count starts from 0. Take care to
 * add one while using this macro to define array sizes.
 * Support only the first 3 affinity levels for now.
 */
#define MPIDR_MAX_AFFLVL    (2)

#define MPID_MASK        (MPIDR_MT_MASK                 | \
                (MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT) | \
                 (MPIDR_AFFLVL_MASK << MPIDR_AFF2_SHIFT) | \
                 (MPIDR_AFFLVL_MASK << MPIDR_AFF1_SHIFT) | \
                 (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT))

/* Define function for system instruction with type specifier */
#define DEFINE_SYSOP_TYPE_FUNC(_op, _type)               \
static inline void _op ## _type(void)                    \
{                                                        \
    __asm__ (#_op " " #_type);                           \
}

#endif /* ARCH_HELPERS_H */
