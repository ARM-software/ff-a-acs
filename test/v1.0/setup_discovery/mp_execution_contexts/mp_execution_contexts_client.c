/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define DATA_PATTERN 0x44332211

static event_t cpu_booted[32];

uint32_t mp_execution_contexts_sec_cpu_client(uint32_t test_num)
{
    ffa_args_t payload;
    uint64_t mpid = val_read_mpidr() & MPID_MASK;
    val_endpoint_info_t *ep_info = val_get_endpoint_info();
    uint32_t test_run_data = TEST_RUN_DATA(test_num,
                                  VM1,
                                  SP1,
                                  CLIENT_TEST);

    LOG(DBG, "Secondary cpu with mpid 0x%x booted\n", mpid);

    /* Skip the mp direct messaging if SP1 is EL0 SP (UP SP) */
    if (ep_info[SP1].ec_count != 1)
    {
        val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

        /* Rule - The Receiver could be implemented as a MP endpoint. In this case,
         * the number of execution contexts that the endpoint implements must
         * be equal to the number of PEs in the system.
         * - Send direct request to SP1 (MP EP) on all secondary cpus
         */
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = ((uint32_t)val_get_endpoint_id(VM1) << 16) |
                                val_get_endpoint_id(SP1);
        payload.arg3 = DATA_PATTERN;
        val_ffa_msg_send_direct_req_64(&payload);

        if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_64)
        {
            LOG(ERROR, "Direct request failed, fid=0x%x, err %x\n",
                      payload.fid, payload.arg2);
            return VAL_ERROR_POINT(1);
        }

        if (payload.arg3 != DATA_PATTERN)
        {
            LOG(ERROR, "data mismatch expected=0x%x, actual=0x%x\n",
                      DATA_PATTERN, payload.arg3);
            return VAL_ERROR_POINT(2);
        }

        payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

        LOG(DBG, "Direct msg passed for mpid=%x\n", mpid);

        /* Tell the boot CPU that the calling CPU has completed the test */
        val_send_event(&cpu_booted[val_get_cpuid(mpid)]);

        return (uint32_t)payload.arg3;
    }

    /* Tell the boot CPU that the calling CPU has completed the test */
    val_send_event(&cpu_booted[val_get_cpuid(mpid)]);

    return VAL_SUCCESS;
}

uint32_t mp_execution_contexts_client(uint32_t test_run_data)
{
    uint32_t i, total_cpus = val_get_no_of_cpus(), ret;
    uint64_t boot_mpid, mpid;
    uint32_t test_num = GET_TEST_NUM(test_run_data);
    uint32_t status = VAL_ERROR;

    /* Setup VM1 mp execution contexts */
    boot_mpid = val_read_mpidr() & MPID_MASK;

    for (i = 0; i < total_cpus; i++)
    {
        val_init_event(&cpu_booted[i]);
    }

    LOG(DBG, "boot cpu mpid %x\n", boot_mpid);

    for (i = 0; i < total_cpus; i++)
    {
        mpid = val_get_mpid(i);

        if (mpid == boot_mpid)
        {
            continue;
        }

        LOG(DBG, "Power up secondary CPU mpid=%x\n", mpid);
        ret = val_power_on_cpu(i);
        if (ret != 0)
        {
            LOG(ERROR, "val_power_on_cpu mpid 0x%x returns %x\n", mpid, ret);
            return VAL_ERROR_POINT(3);
        }

        val_wait_for_event(&cpu_booted[i]);
        LOG(DBG, "Powered off secondary CPU mpid=%x\n", mpid);
    }



    for (i = 0; i < total_cpus; i++)
    {
        mpid = val_get_mpid(i);

        if (mpid == boot_mpid)
        {
            continue;
        }

        /* Get MP Test Status */
        status = val_get_multi_pe_test_status(mpid, test_num);
        if (status != VAL_SUCCESS)
        {
            return status;
        }
    }

    /* Unused argument */
    (void)test_run_data;
    return VAL_SUCCESS;
}
