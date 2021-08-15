/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <string.h>
#include <pal_arch_helpers.h>
#include <pal_arm_gic.h>
#include <pal_interfaces.h>
#include <pal_irq.h>

/*
 * On FVP, consider that the last SPI is the Trusted Random Number Generator
 * interrupt.
 */
#define PLAT_MAX_SPI_OFFSET_ID      107

#define IS_PLAT_SPI(irq_num)                        \
    (((irq_num) >= MIN_SPI_ID) &&                    \
     ((irq_num) <= MIN_SPI_ID + PLAT_MAX_SPI_OFFSET_ID))

static spi_desc spi_desc_table[PLAT_MAX_SPI_OFFSET_ID + 1];
static ppi_desc ppi_desc_table[PLATFORM_NO_OF_CPUS][
                (MAX_PPI_ID + 1) - MIN_PPI_ID];
static sgi_desc sgi_desc_table[PLATFORM_NO_OF_CPUS][MAX_SGI_ID + 1];
static spurious_desc spurious_desc_handler;

/*
 * For a given SPI, the associated IRQ handler is common to all CPUs.
 * Therefore, we need a lock to prevent simultaneous updates.
 *
 * We use one lock for all SPIs. This will make it impossible to update
 * different SPIs' handlers at the same time (although it would be fine) but it
 * saves memory. Updating an SPI handler shouldn't occur that often anyway so we
 * shouldn't suffer from this restriction too much.
 */
static s_lock_t spi_lock;

static handler_irq_t *get_irq_handler(int irq_num)
{
    if (IS_PLAT_SPI(irq_num))
        return &spi_desc_table[irq_num - MIN_SPI_ID].handler;

    unsigned int mpid = (uint32_t)read_mpidr_el1();
    unsigned int linear_id = pal_get_cpuid(mpid);

    if (IS_PPI(irq_num))
        return &ppi_desc_table[linear_id][irq_num - MIN_PPI_ID].handler;

    if (IS_SGI(irq_num))
        return &sgi_desc_table[linear_id][irq_num - MIN_SGI_ID].handler;

    /*
     * The only possibility is for it to be a spurious
     * interrupt.
     */
    assert(irq_num == GIC_SPURIOUS_INTERRUPT);
    return &spurious_desc_handler;
}

void pal_send_sgi(int sgi_id, unsigned int core_pos)
{
    assert(IS_SGI(sgi_id));

    /*
     * Ensure that all memory accesses prior to sending the SGI are
     * completed.
     */
    dsbish();

    arm_gic_send_sgi(sgi_id, core_pos);
}

void pal_irq_enable(int irq_num, uint8_t irq_priority)
{
    if (IS_PLAT_SPI(irq_num)) {
        /*
         * Instruct the GIC Distributor to forward the interrupt to
         * the calling core
         */
        arm_gic_set_intr_target(irq_num, pal_get_cpuid(read_mpidr_el1()));
    }

    arm_gic_set_intr_priority(irq_num, irq_priority);
    arm_gic_intr_enable(irq_num);

    pal_printf("Enabled IRQ #%u\n", (uint64_t)irq_num, 0);
}

void pal_irq_disable(int irq_num)
{
    /* Disable the interrupt */
    arm_gic_intr_disable(irq_num);

    pal_printf("Disabled IRQ #%u\n", (uint64_t)irq_num, 0);
}

#define HANDLER_VALID(handler, expect_handler)        \
    ((expect_handler) ? ((handler) != NULL) : ((handler) == NULL))

static int pal_irq_update_handler(int irq_num,
                   handler_irq_t irq_handler,
                   bool expect_handler)
{
    handler_irq_t *cur_handler;
    int ret = -1;

    cur_handler = get_irq_handler(irq_num);
    if (IS_PLAT_SPI(irq_num))
        pal_spin_lock(&spi_lock);

    /*
     * Update the IRQ handler, if the current handler is in the expected
     * state
     */
    assert(HANDLER_VALID(*cur_handler, expect_handler));
    if (HANDLER_VALID(*cur_handler, expect_handler))
    {
        *cur_handler = irq_handler;
        ret = 0;
    }

    if (IS_PLAT_SPI(irq_num))
        pal_spin_unlock(&spi_lock);

    return ret;
}

int pal_irq_register_handler(int irq_num, void *irq_handler)
{
    int ret;

    ret = pal_irq_update_handler(irq_num, (handler_irq_t)irq_handler, false);
    if (ret == 0)
        pal_printf("Registered IRQ handler for IRQ #%u\n", (uint64_t)irq_num, 0);

    return ret;
}

int pal_irq_unregister_handler(int irq_num)
{
    int ret;

    ret = pal_irq_update_handler(irq_num, NULL, true);
    if (ret == 0)
        pal_printf("Unregistered IRQ handler for IRQ #%u\n", (uint64_t)irq_num, 0);

    return ret;
}

int pal_irq_handler_dispatcher(void)
{
    unsigned int raw_iar;
    int irq_num;
    sgi_data_t sgi_data;
    handler_irq_t *handler;
    void *irq_data = NULL;
    int rc = 0;

    /* Acknowledge the interrupt */
    irq_num = (int)arm_gic_intr_ack(&raw_iar);

    handler = get_irq_handler(irq_num);
    if (IS_PLAT_SPI(irq_num))
    {
        irq_data = &irq_num;
    } else if (IS_PPI(irq_num))
    {
        irq_data = &irq_num;
    } else if (IS_SGI(irq_num))
    {
        sgi_data.irq_id = (uint32_t)irq_num;
        irq_data = &sgi_data;
    }

    if (*handler != NULL)
        rc = (*handler)(irq_data);

    /* Mark the processing of the interrupt as complete */
    if (irq_num != GIC_SPURIOUS_INTERRUPT)
        arm_gic_end_of_intr(raw_iar);

    return rc;
}

void pal_irq_setup(void)
{
    memset(spi_desc_table, 0, sizeof(spi_desc_table));
    memset(ppi_desc_table, 0, sizeof(ppi_desc_table));
    memset(sgi_desc_table, 0, sizeof(sgi_desc_table));
    memset(&spurious_desc_handler, 0, sizeof(spurious_desc_handler));
    pal_init_spinlock(&spi_lock);
}
