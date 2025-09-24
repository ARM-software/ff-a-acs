/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_xen_pvconsole.h"

#include <xen/hypercall.h>
#include <xen/public/io/console.h>
#include <xen/public/event_channel.h>
#include <xen/public/hvm/hvm_op.h>
#include <xen/public/hvm/params.h>
#include <xen/public/sched.h>

#include "pal_xen_console.h"

#define ACCESS_ONCE(x)          (*(volatile typeof(x) *)&(x))

#define XEN_PAGE_SHIFT          12

struct xencons_interface *cons_ring;
evtchn_port_t cons_evtchn;

static int hvm_get_parameter(uint32_t idx, domid_t domid, uint64_t *value)
{
    int ret = 0;
    struct xen_hvm_param xhv;

    xhv.domid = domid;
    xhv.index = idx;

    ret = HYPERVISOR_hvm_op(HVMOP_get_param, &xhv);
    if (ret < 0) {
        return ret;
    }

    *value = xhv.value;
    return ret;
}

static void driver_xen_pvconsole_init(void)
{
    uint64_t console_pfn, port;
    int ret;

    ret = hvm_get_parameter(HVM_PARAM_CONSOLE_EVTCHN, DOMID_SELF, &port);
    if (ret) {
        return;
    }

    ret = hvm_get_parameter(HVM_PARAM_CONSOLE_PFN, DOMID_SELF, &console_pfn);
    if (ret) {
        return;
    }

    cons_evtchn = (evtchn_port_t)port;

    /* make the assumption we are 1:1 map */
    cons_ring = (struct xencons_interface *)(console_pfn << XEN_PAGE_SHIFT);
}

void driver_xen_pvconsole_putc(char c)
{
    struct evtchn_send send;
    XENCONS_RING_IDX prod,out_idx;

    if (cons_ring == NULL) {
        driver_xen_pvconsole_init();
    }

    if (cons_ring == NULL) {
        driver_xen_console_putc(c);
        return;
    }

    prod = cons_ring->out_prod;

    out_idx = MASK_XENCONS_IDX(prod, cons_ring->out);
    cons_ring->out[out_idx] = c;
    prod++;

	__asm__ __volatile__ ("dmb ishst" : : : "memory");
    ACCESS_ONCE(cons_ring->out_prod) = prod;

    if (c == '\n') {
        send.port = cons_evtchn;
        HYPERVISOR_event_channel_op(EVTCHNOP_send, &send);

        while (ACCESS_ONCE(cons_ring->out_cons) != cons_ring->out_prod)
            HYPERVISOR_sched_op(SCHEDOP_yield, NULL);
    }
}
