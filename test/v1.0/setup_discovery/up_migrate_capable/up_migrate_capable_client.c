/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static event_t cpu_booted;

uint32_t up_migrate_capable_sec_cpu_client(uint32_t test_num)
{
    ffa_args_t payload;
    uint64_t mpid = val_read_mpidr() & MPID_MASK;
    (void)(test_num);

    LOG(DBG, "\tSecondary cpu with mpid 0x%x booted\n", mpid, 0);

    /* Send direct request using secondary cpu */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(VM1) << 16) |
                            val_get_endpoint_id(SP3);
    val_ffa_msg_send_direct_req_32(&payload);

    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err %x\n",
                  payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    LOG(DBG, "\tDirect msg to SP3 passed for mpid=%x\n", mpid, 0);

    /* Tell the boot CPU that the calling CPU has completed the test */
    val_send_event(&cpu_booted);

    return VAL_SUCCESS;
}

uint32_t up_migrate_capable_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t i, total_cpus = val_get_no_of_cpus(), ret;
    uint64_t boot_mpid, mpid = 0;
    uint32_t status = VAL_ERROR;
    uint32_t test_num = GET_TEST_NUM(test_run_data);

    /* Run server test on boot cpu */
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err %x\n",
                  payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    /* Setup VM1 another execution context */
    boot_mpid = val_read_mpidr() & MPID_MASK;

    val_init_event(&cpu_booted);

    LOG(DBG, "\tboot cpu mpid %x\n", boot_mpid, 0);
    for (i = 0; i < total_cpus; i++)
    {
        mpid = val_get_mpid(i);

        LOG(DBG, "\tPower up secondary CPUs mpid=%x\n", mpid, 0);
        if (mpid == boot_mpid)
        {
            continue;
        }

        ret = val_power_on_cpu(i);
        if (ret != 0)
        {
            LOG(ERROR, "\tval_power_on_cpu mpid 0x%x returns %x\n", mpid, ret);
            return VAL_ERROR_POINT(2);
        }
        break;
    }

    LOG(DBG, "\tWaiting secondary CPU to turn off ...\n", 0, 0);

    val_wait_for_event(&cpu_booted);

    LOG(DBG, "\tCPU=%x power off ...\n", mpid, 0);

    /* Collect the server status in payload.arg3 */
    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    /* Get MP Test Status */
    status = val_get_multi_pe_test_status(mpid, test_num);

    return status ? status : (uint32_t)payload.arg3;
}
