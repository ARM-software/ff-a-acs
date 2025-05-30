/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "val_def.h"

IMAGE_BASE = PLATFORM_SP_IMAGE_OFFSET;
OUTPUT_FORMAT(elf64-littleaarch64)
OUTPUT_ARCH(aarch64)
ENTRY(val_entry)

PHDRS
{
    text PT_LOAD FLAGS(RX);    /* Code, read + execute */
    rodata PT_LOAD FLAGS(R);   /* Read-only data */
    data PT_LOAD FLAGS(RW);    /* Writable data */
    bss PT_LOAD FLAGS(RW);     /* BSS (no load, but RW in memory) */
}

SECTIONS
{
    . = IMAGE_BASE;

    ASSERT(. == ALIGN(PLATFORM_PAGE_SIZE),
           "TEXT_START address is not aligned to PAGE_SIZE.")
    
    .text : {
        __TEXT_START__ = .;
        *val_entry.S.o(.text*)
        *(.text*)
        . = NEXT(PLATFORM_PAGE_SIZE);
        __TEXT_END__ = .;
    } :text

    .rodata : {
        . = ALIGN(PLATFORM_PAGE_SIZE);
        __RODATA_START__ = .;
        *(.rodata*)
        . = NEXT(PLATFORM_PAGE_SIZE);
        __RODATA_END__ = .;
    } :rodata

    .data : {
        . = ALIGN(PLATFORM_PAGE_SIZE);
        __DATA_START__ = .;
        *(.data*)
        . = ALIGN(8);
        __GOT_START__ = .;
        *(.got)
        __GOT_END__ = .;
        . = NEXT(PLATFORM_PAGE_SIZE);
        __DATA_END__ = .;
    } :data

    .bss (NOLOAD) : {
        . = ALIGN(PLATFORM_PAGE_SIZE);
        __BSS_START__ = .;
        *(SORT_BY_ALIGNMENT(.bss*))
        *(COMMON)
        . = NEXT(PLATFORM_PAGE_SIZE);
        __BSS_END__ = .;
    } :bss
}