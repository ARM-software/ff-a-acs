/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_interfaces.h"
#include "val_shemaphore.h"
#include "val_vcpu_setup.h"

void val_init_event(event_t *event)
{
    if (event == NULL) {
        LOG(ERR, "val_init_event: event is NULL\n", 0, 0);
        return;
    }
    event->cnt = 0;
    event->lock.lock = 0;
}

void val_init_spinlock(s_lock_t *lock)
{
#ifndef TARGET_LINUX
    pal_init_spinlock(lock);
#endif
}

void val_spin_lock(s_lock_t *lock)
{
#ifndef TARGET_LINUX
    pal_spin_lock(lock);
#endif
}

void val_spin_unlock(s_lock_t *lock)
{
#ifndef TARGET_LINUX
    pal_spin_unlock(lock);
#endif
}

static void send_event_common(event_t *event, unsigned int inc)
{
    if (event == NULL) {
        LOG(ERR, "send_event_common: event is NULL\n", 0, 0);
        return;
    }
    val_spin_lock(&event->lock);
    event->cnt += inc;
    val_spin_unlock(&event->lock);

    val_dataCacheCleanInvalidateVA((uint64_t)&event->cnt);
    /*
     * Make sure the cnt increment is observable by all CPUs
     * before the event is sent.
     */
    dsbsy();
    sev();
}

void val_send_event(event_t *event)
{
    if (event == NULL) {
        LOG(ERR, "val_send_event: event is NULL\n", 0, 0);
        return;
    }
    LOG(DBG, "Sending event %x\n", (uint64_t) event, 0);
    send_event_common(event, 1);
}

void val_send_event_to_all(event_t *event)
{
    if (event == NULL) {
        LOG(ERR, "val_send_event_to_all: event is NULL\n", 0, 0);
        return;
    }
    //LOG("Sending event %p to all CPUs\n", (void *) event);
    send_event_common(event, val_get_no_of_cpus());
}

void val_send_event_to(event_t *event, unsigned int cpus_count)
{
    if (event == NULL) {
        LOG(ERR, "val_send_event_to: event is NULL\n", 0, 0);
        return;
    }
    //LOG("Sending event %p to %u CPUs\n", (void *) event, cpus_count);
    send_event_common(event, cpus_count);
}

void val_wait_for_event(event_t *event)
{
    if (event == NULL) {
        LOG(ERR, "val_wait_for_event: event is NULL\n", 0, 0);
        return;
    }
    unsigned int event_received = 0;

    LOG(DBG, "Waiting for event %x\n", (uint64_t) event, 0);
    while (!event_received) {

        val_dataCacheInvalidateVA((uint64_t)&event->cnt);
        dsbsy();
        /* Wait for someone to send an event */
        if (!event->cnt) {
            wfe();
        } else {
            val_spin_lock(&event->lock);

             /*
              * Check that the event is still pending and that no
              * one stole it from us while we were trying to
              * acquire the lock.
              */
            if (event->cnt != 0) {
                event_received = 1;
                --event->cnt;
            }
            /*
             * No memory barrier is needed here because val_spin_unlock()
             * issues a Store-Release instruction, which guarantees
             * that loads and stores appearing in program order
             * before the Store-Release are observed before the
             * Store-Release itself.
             */
            val_spin_unlock(&event->lock);
        }
    }

    LOG(DBG, "Event recieved for %x\n", (uint64_t) event, 0);
}
