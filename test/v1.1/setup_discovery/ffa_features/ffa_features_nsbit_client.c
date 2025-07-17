/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"


static uint32_t ffa_features_nsbit_helper(uint32_t test_run_data, uint32_t fid)
{
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_args_t payload;
    uint32_t data = 0;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = fid;

    val_ffa_features(&payload);

    if (VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(sender)))
    {
        /* w2[1] A v1.1 SP must set Bit[1] in the Input properties parameter */
        if (payload.fid != FFA_ERROR_32)
        {
            LOG(ERROR, "Must Fail for SP - invalid NS-Bit usage err %x\n", payload.arg2);
            return VAL_ERROR_POINT(1);
        }

        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = fid;
        /* set  NS-Bit - valid usage */
        payload.arg2 = VAL_SET_BITS(payload.arg2, 1, 1, 1);

        val_ffa_features(&payload);
    }

    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "FFA Feature query failed err %x feature fid %x \n", payload.arg2, fid);
        return (payload.arg2 == FFA_ERROR_NOT_SUPPORTED) ? VAL_SKIP_CHECK : VAL_ERROR_POINT(2);
    }

    /* A v1.1 SPMC always sets Bit[1] in the Interface properties return parameter */
    /* Check for ns-bit usage by spmc */
    data = VAL_EXTRACT_BITS(payload.arg2, 1, 1);
    if (data != 1)
    {
        LOG(ERROR, "v1.1 SPMC must set ns-bit for  FFA_MEM_RETRIEVE_REQ val\n");
        return VAL_ERROR_POINT(3);
    }
    LOG(DBG, "ns-bit val %x\n", data);

    return VAL_SUCCESS;
}

uint32_t ffa_features_nsbit_client(uint32_t test_run_data)
{
    uint32_t status, status_1, status_2;

    status_1 = ffa_features_nsbit_helper(test_run_data, FFA_MEM_RETRIEVE_REQ_32);
    status_2 = ffa_features_nsbit_helper(test_run_data, FFA_MEM_RETRIEVE_REQ_64);
    status = (status_2 == VAL_SKIP_CHECK) ?  status_1 : status_2;

    /* Either of the messaging method must be supported */
    if ((status_1 == VAL_SKIP_CHECK) && (status_2 == VAL_SKIP_CHECK))
    {
        LOG(ERROR, "Either of the messaging method must be supported\n");
        return VAL_ERROR_POINT(4);
    } else
        return status;
}
