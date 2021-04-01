/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_direct_message_error1_client(uint32_t test_run_data)
{
    val_endpoint_info_t *ep_info;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_args_t payload;
    uint32_t i;

    ep_info = val_get_endpoint_info();
    if (!ep_info)
    {
        LOG(ERROR, "\tget_endpoint_info error!\n", 0, 0);
        return VAL_ERROR_POINT(1);
    }

    /* Send a direct message to a VM that only supports indirect messaging must be
     * rejected by the Hypervisor
     */
    for (i = 1; i < (VAL_TOTAL_EP_COUNT + 1); i++)
    {
        if (i == client_logical_id)
            continue;

        if (ep_info[i].ep_properties & FFA_INDIRECT_MESSAGE_SUPPORT)
            break;
    }

    if (i == (VAL_TOTAL_EP_COUNT + 1))
    {
        LOG(TEST, "\tSkipping the check, required endpoint not found\n", 0, 0);
        return VAL_SKIP_CHECK;
    }

    if (!val_is_ffa_feature_supported(FFA_MSG_SEND_DIRECT_REQ_32))
    {
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = val_get_endpoint_id(client_logical_id | (uint32_t)ep_info[i].id << 16);
        LOG(DBG, "\tSending direct req to epid=0x%x\n", ep_info[i].id, 0);
        val_ffa_msg_send_direct_req_32(&payload);
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
        {
            LOG(ERROR, "\tUnexpected return status, fid=0x%x, err=0x%x\n",
                payload.fid, payload.arg2);
            return VAL_ERROR_POINT(2);
        }
    }

    /* Send a direct message to a VM that only supports indirect messaging must be
     * rejected by the Hypervisor
     */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = val_get_endpoint_id(client_logical_id | (uint32_t)ep_info[i].id << 16);
    LOG(DBG, "\tSending direct req to epid=0x%x\n", ep_info[i].id, 0);
    val_ffa_msg_send_direct_req_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "\tUnexpected return status, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        return VAL_ERROR_POINT(3);
    }

   if (!val_is_ffa_feature_supported(FFA_MSG_SEND_DIRECT_RESP_32))
    {
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = val_get_endpoint_id(client_logical_id | (uint32_t)ep_info[i].id << 16);
        LOG(DBG, "\tSending direct resp to epid=0x%x\n", ep_info[i].id, 0);
        val_ffa_msg_send_direct_resp_32(&payload);
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
        {
            LOG(ERROR, "\tUnexpected return status, fid=0x%x, err=0x%x\n",
                payload.fid, payload.arg2);
            return VAL_ERROR_POINT(4);
        }
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = val_get_endpoint_id(client_logical_id | (uint32_t)ep_info[i].id << 16);
    LOG(DBG, "\tSending direct resp to epid=0x%x\n", ep_info[i].id, 0);
    val_ffa_msg_send_direct_resp_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "\tUnexpected return status, fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        return VAL_ERROR_POINT(5);
    }

    return VAL_SUCCESS;
}
