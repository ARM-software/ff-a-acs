/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

  .section .text.sysreg, "ax"

    .global val_mair_write
val_mair_write:
    cmp        x1, #2
    b.ne    mair_el1_write
    msr     mair_el2, x0
    ret

mair_el1_write:
    msr     mair_el1, x0
    ret

    .global val_tcr_write
val_tcr_write:
    cmp        x1, #2
    b.ne    tcr_el1_write
    msr     tcr_el2, x0
    tlbi    alle2
    dsb     sy
    isb
    ret
tcr_el1_write:
    msr     tcr_el1, x0
    tlbi     vmalle1
    dsb      sy
    isb
    ret

    .global val_ttbr0_write
val_ttbr0_write:
    cmp        x1, #2
    b.ne    ttbr0_el1_write
    msr     ttbr0_el2, x0
    ret

ttbr0_el1_write:
    msr     ttbr0_el1, x0
    ret

    .global val_ttbr0_read
val_ttbr0_read:
    cmp        x0, #2
    b.ne    ttbr0_el1_read
    mrs     x0, ttbr0_el2
    ret
ttbr0_el1_read:
    mrs     x0, ttbr0_el1
    ret

    .global val_sctlr_read
val_sctlr_read:
    cmp        x0, #2
    b.ne    sctlr_el1_read
    mrs     x0, sctlr_el2
    isb
    ret
sctlr_el1_read:
    mrs     x0, sctlr_el1
    isb
    ret

    .global val_sctlr_write
val_sctlr_write:
    cmp        x1, #2
    b.ne    sctlr_el1_write
    msr     sctlr_el2, x0
    isb
    ret
sctlr_el1_write:
    msr     sctlr_el1, x0
    isb
    ret

    .global val_esr_el1_read
val_esr_el1_read:
    mrs     x0, esr_el1
    ret

    .global val_elr_el1_read
val_elr_el1_read:
    mrs     x0, elr_el1
    ret

    .global val_elr_el1_write
val_elr_el1_write:
    msr     elr_el1, x0
    ret

    .global val_far_el1_read
val_far_el1_read:
    mrs     x0, far_el1
    ret

    .global val_id_aa64mmfr0_el1_read
val_id_aa64mmfr0_el1_read:
    mrs     x0, id_aa64mmfr0_el1
    ret

    .global val_read_mpidr
val_read_mpidr:
    mrs     x0, mpidr_el1
    ret

    .global val_read_current_el
val_read_current_el:
    mrs     x0, CurrentEL
    ret

    .global val_dataCacheCleanInvalidateVA
val_dataCacheCleanInvalidateVA:
    dc  civac, x0
    dsb sy
    isb
    ret

    .global val_dataCacheCleanVA
val_dataCacheCleanVA:
    dc  cvac, x0
    dsb ish
    isb
    ret

    .global val_dataCacheInvalidateVA
val_dataCacheInvalidateVA:
    dc  ivac, x0
    dsb ish
    isb
    ret
