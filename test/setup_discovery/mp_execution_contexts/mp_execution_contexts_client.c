/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define DATA_PATTERN 0x44332211

static event_t cpu_booted[32];

void mp_execution_contexts_sec_cpu(void)
{
    ffa_args_t payload;
    uint64_t mpid = val_read_mpidr() & MPID_MASK;
    val_endpoint_info_t *ep_info = val_get_endpoint_info();

    LOG(DBG, "\tSecondary cpu with mpid 0x%x booted\n", mpid, 0);
    /* Skip the mp direct messaging if SP1 is EL0 SP (UP SP) */
    if (ep_info[SP1].ec_count != 1)
    {
        /* Rule - The Receiver could be implemented as a MP endpoint. In this case,
         * the number of execution contexts that the endpoint implements must
         * be equal to the number of PEs in the system.
         * - Send direct request to SP1 (MP EP) on all secondary cpus
         */
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = ((uint32_t)val_get_endpoint_id(VM1) << 16) |
                                val_get_endpoint_id(SP1);
        payload.arg3 = DATA_PATTERN;
        val_ffa_msg_send_direct_req_32(&payload);

        if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
        {
            LOG(ERROR, "\tDirect request failed, fid=0x%x, err %x\n",
                      payload.fid, payload.arg2);
            VAL_PANIC("Can't recover\n");
        }

        if (payload.arg3 != DATA_PATTERN)
        {
            LOG(ERROR, "\tdata mismatch expected=0x%x, actual=0x%x\n",
                      DATA_PATTERN, payload.arg3);
            VAL_PANIC("Can't recover\n");
        }
    }

    LOG(DBG, "\tDirect msg passed for mpid=%x\n", mpid, 0);

    /* Tell the boot CPU that the calling CPU has completed the test */
    val_send_event(&cpu_booted[val_get_cpuid(mpid)]);

    val_power_off_cpu();
}

uint32_t mp_execution_contexts_client(uint32_t test_run_data)
{
    uint32_t i, total_cpus = val_get_no_of_cpus(), ret;
    uint64_t boot_mpid, mpid;

    /* Setup VM1 mp execution contexts */
    boot_mpid = val_read_mpidr() & MPID_MASK;

    for (i = 0; i < total_cpus; i++)
    {
        val_init_event(&cpu_booted[i]);
    }

    LOG(DBG, "\tboot cpu mpid %x\n", boot_mpid, 0);
    for (i = 0; i < total_cpus; i++)
    {
        mpid = val_get_mpid(i);

        if (mpid == boot_mpid)
        {
            continue;
        }

        LOG(DBG, "\tPower up secondary CPU mpid=%x\n", mpid, 0);
        ret = val_power_on_cpu(i);
        if (ret != 0)
        {
            LOG(ERROR, "\tval_power_on_cpu mpid 0x%x returns %x\n", mpid, ret);
            return VAL_ERROR_POINT(1);
        }

        val_wait_for_event(&cpu_booted[i]);
        LOG(DBG, "\tPowered off secondary CPU mpid=%x\n", mpid, 0);
    }


    /* Unused argument */
    (void)test_run_data;
    return VAL_SUCCESS;
}
