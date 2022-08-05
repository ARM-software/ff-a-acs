/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t up_migrate_capable_server(ffa_args_t args)
{
    ffa_args_t payload = args;

    /* Is direct request received? */
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err %x\n",
                  payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    /* Release client to send direct req on different PE */
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0,
                                        0, 0, 0, 0);


    /* SP3 is uni-processor capable with a single execution context.
     * It must run only on a single PE in the system at any point of
     * time. Since client has send direct request on different PE other
     * than boot cpu, spm must migrate sp3 on the PE on which client
     * has sent second direct request.
     * */
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
    {
        LOG(ERROR, "\tDirect request failed, fid=0x%x, err %x\n",
                  payload.fid, payload.arg2);
        return VAL_ERROR_POINT(2);
    }

    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0,
                                        0, 0, 0, 0);

    return VAL_SUCCESS;
}
