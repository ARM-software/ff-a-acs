/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_SHEMAPHORE_H_
#define _VAL_SHEMAPHORE_H_

#include "val_vcpu_setup.h"

void val_init_spinlock(s_lock_t *lock);
void val_spin_lock(s_lock_t *lock);
void val_spin_unlock(s_lock_t *lock);

/*
 * Initialise an event.
 *   event: Address of the event to initialise
 *
 * This function can be used either to initialise a newly created event
 * structure or to recycle one.
 *
 * Note: This function is not MP-safe. It can't use the event lock as it is
 * responsible for initialising it. Care must be taken to ensure this function
 * is called in the right circumstances.
 */
void val_init_event(event_t *event);

/*
 * Send an event to a CPU.
 *   event: Address of the variable that acts as a synchronisation object.
 *
 * Which CPU receives the event is determined on a first-come, first-served
 * basis. If several CPUs are waiting for the same event then the first CPU
 * which takes the event will reflect that in the event structure.
 *
 * Note: This is equivalent to calling:
 *   val_send_event_to(event, 1);
 */
void val_send_event(event_t *event);

/*
 * Send an event to all CPUs.
 *   event: Address of the variable that acts as a synchronisation object.
 *
 * Note: This is equivalent to calling:
 *   val_send_event_to(event, PLATFORM_NO_OF_CPUS);
 */
void val_send_event_to_all(event_t *event);

/*
 * Send an event to a given number of CPUs.
 *   event: Address of the variable that acts as a synchronisation object.
 *   cpus_count: Number of CPUs to send the event to.
 *
 * Which CPUs receive the event is determined on a first-come, first-served
 * basis. If more than 'cpus_count' CPUs are waiting for the same event then the
 * first 'cpus_count' CPUs which take the event will reflect that in the event
 * structure.
 */
void val_send_event_to(event_t *event, unsigned int cpus_count);

/*
 * Wait for an event.
 *   event: Address of the variable that acts as a synchronisation object.
 */
void val_wait_for_event(event_t *event);

#endif /* _VAL_SHEMAPHORE_H_ */
