/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PAL_IRQ_H__
#define  __PAL_IRQ_H__

/* Prototype of a handler function for an IRQ */
typedef int (*handler_irq_t)(void *data);

/* Keep track of the IRQ handler registered for a given SPI */
typedef struct {
    handler_irq_t handler;
} spi_desc;

/* Data associated with the reception of an SGI */
typedef struct {
    /* Interrupt ID of the signaled interrupt */
    unsigned int irq_id;
} sgi_data_t;

/* Keep track of the IRQ handler registered for a spurious interrupt */
typedef handler_irq_t spurious_desc;

/*******************************************************************************
 * Used to align variables on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT       6
#define CACHE_WRITEBACK_GRANULE     (1U << CACHE_WRITEBACK_SHIFT)

#ifndef TARGET_LINUX
#define __aligned(x)    __attribute__((__aligned__(x)))
#endif
/*
 * PPIs and SGIs are interrupts that are private to a GIC CPU interface. These
 * interrupts are banked in the GIC Distributor. Therefore, each CPU can
 * set up a different IRQ handler for a given PPI/SGI.
 *
 * So we define a data structure representing an IRQ handler aligned on the
 * size of a cache line. This guarantees that in an array of these, each element
 * is loaded in a separate cache line. This allows efficient concurrent
 * manipulation of these elements on different CPUs.
 */
typedef struct {
    handler_irq_t handler;
} __aligned(CACHE_WRITEBACK_GRANULE)irq_handler_banked_t;

typedef irq_handler_banked_t ppi_desc;
typedef irq_handler_banked_t sgi_desc;

#endif /* __PAL_IRQ_H__ */
