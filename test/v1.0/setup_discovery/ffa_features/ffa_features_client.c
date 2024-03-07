/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define FFA_INVALID_FID 0x8FFFFFFF

static uint32_t ffa_feature_query(uint32_t fid, char *str)
{
    ffa_args_t payload;
    uint32_t data;
    uint32_t data1;
    uint32_t output_reserve_count;
    uint32_t status = VAL_SUCCESS;
    char string[PRINT_LIMIT] = "";

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = fid;
#if (PLATFORM_FFA_V_1_1 == 1 || PLATFORM_FFA_V_ALL == 1)
    if (fid == FFA_MEM_RETRIEVE_REQ_32)
    {
        payload.arg2 = 0x2;
    }
#endif
    val_ffa_features(&payload);

    val_strcat(string, str, sizeof(string));
    if (payload.fid == FFA_ERROR_32 && (payload.arg2 == FFA_ERROR_NOT_SUPPORTED))
    {
        val_strcat(string, " ->feature not supported\n", sizeof(string));
        LOG(DBG, string, 0, 0);
    }
    else if (payload.fid == FFA_SUCCESS_32 || payload.fid == FFA_SUCCESS_64)
    {
        val_strcat(string, " ->feature supported\n", sizeof(string));
        LOG(DBG, string, 0, 0);
    }
    else
    {
        val_strcat(string, " -Invalid return code received, fid=%x, err=%x\n", sizeof(string));
        LOG(ERROR, string, payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

    switch (fid)
    {
        /* Mandatory FFA features must be supported */
        case FFA_ERROR_32:
        case FFA_INTERRUPT_32:
        case FFA_VERSION_32:
        case FFA_FEATURES_32:
        case FFA_RX_RELEASE_32:
        case FFA_PARTITION_INFO_GET_32:
        case FFA_ID_GET_32:
        case FFA_MSG_WAIT_32:
        case FFA_RXTX_UNMAP_32:
        case FFA_RUN_32:
            if (payload.fid == FFA_ERROR_32)
            {
                LOG(ERROR, "\tfid = 0x%x must be supported\n", fid, 0);
                status = VAL_ERROR_POINT(2);
                break;
            }
            /* Check output w2-w7 reserved(MBZ) */
            output_reserve_count = 6;
            if (val_reserve_param_check(payload, output_reserve_count))
            {
                LOG(ERROR, "\treserved registers must be zero\n", 0, 0);
                status = VAL_ERROR_POINT(3);
            }

            break;

        /* FF-A mandatory features - Atleast either of
         * interface must be supported */
        case FFA_SUCCESS_32:
        case FFA_SUCCESS_64:
            if (payload.fid == FFA_ERROR_32)
            {
                status = VAL_SKIP_CHECK;
                break;
            }
            /* Check output w2-w7 reserved(MBZ) */
            output_reserve_count = 6;
            if (val_reserve_param_check(payload, output_reserve_count))
            {
                LOG(ERROR, "\treserved registers must be zero\n", 0, 0);
                status = VAL_ERROR_POINT(4);
            }
            break;

        case FFA_RXTX_MAP_64:
        case FFA_RXTX_MAP_32:
            if (payload.fid == FFA_ERROR_32)
            {
                status = VAL_SKIP_CHECK;
                break;
            }
            /* Check RXTX_MAP alignment boundary w2[1:0] */
            data = VAL_EXTRACT_BITS(payload.arg2, 0, 1);
            if (data == FFA_RXTX_MAP_4K_SIZE)
            {
                LOG(DBG, "\tRXTX_MAP alignment boundary is 4K size\n", 0, 0);
            }
            else if (data == FFA_RXTX_MAP_64K_SIZE)
            {
                LOG(DBG, "\tRXTX_MAP alignment boundary is 64K size\n", 0, 0);
            }
            else if (data == FFA_RXTX_MAP_16K_SIZE)
            {
                LOG(DBG, "\tRXTX_MAP alignment boundary is 16K size\n", 0, 0);
            }
            else
            {
                LOG(ERROR, "\tFFA_RXTX_MAP_64 alignment boundary not defined\n", 0, 0);
                status = VAL_ERROR_POINT(5);
                break;
            }

            /* Output w2[31:2] and w3 are reserved(MBZ) */
            data = VAL_EXTRACT_BITS(payload.arg2, 2, 31);
            if (data)
            {
                LOG(ERROR, "\tw2[31:2] must be zero\n", 0, 0);
                status = VAL_ERROR_POINT(6);
                break;
            }
            /* Check output w3-w7 reserved(MBZ) */
            output_reserve_count = 5;
            if (val_reserve_param_check(payload, output_reserve_count))
            {
                LOG(ERROR, "\treserved registers must be zero\n", 0, 0);
                status = VAL_ERROR_POINT(7);
            }
            break;

        /* FF-A optional features */
        case FFA_MSG_POLL_32:
        case FFA_YIELD_32:
        case FFA_MSG_SEND_32:
        case FFA_MSG_SEND_DIRECT_REQ_32:
        case FFA_MSG_SEND_DIRECT_RESP_32:
        case FFA_MSG_SEND_DIRECT_REQ_64:
        case FFA_MSG_SEND_DIRECT_RESP_64:
        case FFA_MEM_RETRIEVE_RESP_32:
        case FFA_MEM_RELINQUISH_32:
        case FFA_MEM_RECLAIM_32:
            if (payload.fid == FFA_ERROR_32)
            {
                status = VAL_SKIP_CHECK;
                break;
            }
            /* Check output w2-w7 reserved(MBZ) */
            output_reserve_count = 6;
            if (val_reserve_param_check(payload, output_reserve_count))
            {
                LOG(ERROR, "\treserved registers must be zero\n", 0, 0);
                status = VAL_ERROR_POINT(8);
            }
            break;

        case FFA_MEM_SHARE_32:
        case FFA_MEM_LEND_32:
        case FFA_MEM_DONATE_32:
        case FFA_MEM_SHARE_64:
        case FFA_MEM_LEND_64:
        case FFA_MEM_DONATE_64:
            if (payload.fid == FFA_ERROR_32)
            {
                status = VAL_SKIP_CHECK;
                break;
            }
            /* Check for Dynamically allocated buffer support w2[0] */
            data = VAL_EXTRACT_BITS(payload.arg2, 0, 0);
            LOG(DBG, str, 0, 0);
            if (data == FFA_DYNAMIC_BUFFER_SUPPORT)
            {
                LOG(DBG, "\tDynamic buffer supported\n", 0, 0);
            }
            else
            {
                LOG(DBG, "\tDynamic buffer not supported\n", 0, 0);
            }

            /* Output w2[31:1] and w3 are reserved(MBZ) */
            data = VAL_EXTRACT_BITS(payload.arg2, 1, 31);
            if (data || payload.arg3)
                status = VAL_ERROR_POINT(9);
            break;

        case FFA_MEM_RETRIEVE_REQ_64:
        case FFA_MEM_RETRIEVE_REQ_32:
            if (payload.fid == FFA_ERROR_32)
            {
                status = VAL_SKIP_CHECK;
                break;
            }
            /* Check for Dynamically allocated buffer support w2[0] */
            data = VAL_EXTRACT_BITS(payload.arg2, 0, 0);
            LOG(DBG, str, 0, 0);
            if (data == FFA_DYNAMIC_BUFFER_SUPPORT)
            {
                LOG(DBG, "\tDynamic buffer supported\n", 0, 0);
            }
            else
            {
                LOG(DBG, "\tDynamic buffer not supported\n", 0, 0);
            }

            /* Check for Outstanding retrievals field w3[7:0] */
            data = VAL_EXTRACT_BITS(payload.arg3, 0, 7);
            LOG(DBG, str, 0, 0);
            LOG(DBG, "\tOutstanding retrievals count %d\n", data, 0);

            /* Output w2[31:1] and w3[31:8] are reserved(MBZ) */
#if (PLATFORM_FFA_V_1_0 == 1)
            data = VAL_EXTRACT_BITS(payload.arg2, 1, 31);
#else
            data = VAL_EXTRACT_BITS(payload.arg2, 3, 31);
#endif
            data1 = VAL_EXTRACT_BITS(payload.arg3, 8, 31);
            if (data || data1)
                status = VAL_ERROR_POINT(10);
            break;

        default:
            LOG(INFO, "\tfid=%x invalid feature \n", fid, 0);
            status = VAL_SKIP_CHECK;
    }

    return status;
}

uint32_t ffa_features_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status_1, status_2, status_3, status_4, status_5, status_6, status_7;
    uint32_t status_8, status_9, status_10, status_11;
    uint32_t output_reserve_count;
    uint32_t messaging_type;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    val_endpoint_info_t *ep_info;

    ep_info = val_get_endpoint_info();
    if (!ep_info)
    {
        LOG(ERROR, "\tget_endpoint_info error!\n", 0, 0);
        return VAL_ERROR_POINT(11);
    }

    messaging_type = ep_info[client_logical_id].ep_properties;

    status_1 = ffa_feature_query(FFA_ERROR_32, "\tFFA_ERROR_32");
    if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_SUCCESS_32, "\tFFA_SUCCESS_32");
    status_2 = ffa_feature_query(FFA_SUCCESS_64, "\tFFA_SUCCESS_64");

    if ((status_1 && status_2) ||
        (status_1 == VAL_ERROR_POINT(1)) ||
        (status_2 == VAL_ERROR_POINT(1)))
        return VAL_ERROR_POINT(12);

    status_1 = ffa_feature_query(FFA_INTERRUPT_32, "\tFFA_INTERRUPT_32");
    if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_VERSION_32, "\tFFA_VERSION_32");
    if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_FEATURES_32, "\tFFA_FEATURES_32");
    if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_RX_RELEASE_32, "\tFFA_RX_RELEASE_32");
    if (status_1)
        return status_1;

    val_reprogram_watchdog();

    status_1 = ffa_feature_query(FFA_RXTX_UNMAP_32, "\tFFA_RXTX_UNMAP_32");
    if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_PARTITION_INFO_GET_32, "\tFFA_PARTITION_INFO_GET_32");
    if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_ID_GET_32, "\tFFA_ID_GET_32");
    if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_MSG_WAIT_32, "\tFFA_MSG_WAIT_32");
    if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_RUN_32, "\tFFA_RUN_32");
    if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_RXTX_MAP_32, "\tFFA_RXTX_MAP_32");
    status_2 = ffa_feature_query(FFA_RXTX_MAP_64, "\tFFA_RXTX_MAP_64");

    if ((status_1 && status_2) ||
        (status_1 == VAL_ERROR_POINT(1)) ||
        (status_2 == VAL_ERROR_POINT(1)))
        return VAL_ERROR_POINT(13);

    val_reprogram_watchdog();

    status_1 = ffa_feature_query(FFA_MEM_SHARE_64, "\tFFA_MEM_SHARE_64");
    status_2 = ffa_feature_query(FFA_MEM_SHARE_32, "\tFFA_MEM_SHARE_32");
    status_3 = ffa_feature_query(FFA_MEM_LEND_64, "\tFFA_MEM_LEND_64");
    status_4 = ffa_feature_query(FFA_MEM_LEND_32, "\tFFA_MEM_LEND_32");
    status_5 = ffa_feature_query(FFA_MEM_DONATE_64, "\tFFA_MEM_DONATE_64");
    status_6 = ffa_feature_query(FFA_MEM_DONATE_32, "\tFFA_MEM_DONATE_32");

    /* At least one of the ABI must be supported - share/donate/lend */
    if (status_1 && status_2 && status_3
         && status_4 && status_5 && status_6)
    {
           LOG(ERROR, "\tAt least one of the memory manage ABI must be supported\n", 0, 0)
           return VAL_ERROR_POINT(14);
    }

    val_reprogram_watchdog();

    status_7 = ffa_feature_query(FFA_MEM_RETRIEVE_REQ_64, "\tFFA_MEM_RETRIEVE_REQ_64");
    status_8 = ffa_feature_query(FFA_MEM_RETRIEVE_REQ_32, "\tFFA_MEM_RETRIEVE_REQ_32");
    status_9 = ffa_feature_query(FFA_MEM_RETRIEVE_RESP_32, "\tFFA_MEM_RETRIEVE_RESP_32");
    status_10 = ffa_feature_query(FFA_MEM_RELINQUISH_32, "\tFFA_MEM_RELINQUISH_32");
    status_11 = ffa_feature_query(FFA_MEM_RECLAIM_32, "\tFFA_MEM_RECLAIM_32");

    /* If 64 bit manage ABI supported */
    if (!status_1 || !status_3 || !status_5)
    {
        if (status_7 || status_9 || status_10 || status_11)
           return VAL_ERROR_POINT(15);
    }

    /* If 32 bit manage ABI supported */
    if (!status_2 || !status_4 || !status_6)
    {
        if (status_8 || status_9 || status_10 || status_11)
           return VAL_ERROR_POINT(16);
    }

    val_reprogram_watchdog();

    status_1 = ffa_feature_query(FFA_MSG_SEND_32, "\tFFA_MSG_SEND_32");
    status_2 = ffa_feature_query(FFA_MSG_POLL_32, "\tFFA_MSG_POLL_32");
    status_3 = ffa_feature_query(FFA_YIELD_32, "\tFFA_YIELD_32");

    /* Cross check with manifest field value. Following must be
     * supported if indirect messaging is supported */
    if ((messaging_type & FFA_INDIRECT_MESSAGE_SUPPORT) &&
            (val_get_curr_endpoint_id() != HYPERVISOR_ID))
    {
        if (status_1 || status_2 || status_3)
        {
            LOG(ERROR, "\tInvalid return code for indirect messaging ABIs\n", 0, 0)
            return VAL_ERROR_POINT(17);
        }
    }
    else
    {
        if (!status_1 || !status_2 || !status_3)
        {
            LOG(ERROR, "\tInvalid return code for indirect messaging ABIs\n", 0, 0)
            return VAL_ERROR_POINT(18);
        }
    }

    val_reprogram_watchdog();

    status_2 = ffa_feature_query(FFA_MSG_SEND_DIRECT_REQ_64, "\tFFA_MSG_SEND_DIRECT_REQ_64");
    status_3 = ffa_feature_query(FFA_MSG_SEND_DIRECT_RESP_64, "\tFFA_MSG_SEND_DIRECT_RESP_64");
    status_4 = ffa_feature_query(FFA_MSG_SEND_DIRECT_REQ_32, "\tFFA_MSG_SEND_DIRECT_REQ_32");
    status_5 = ffa_feature_query(FFA_MSG_SEND_DIRECT_RESP_32, "\tFFA_MSG_SEND_DIRECT_RESP_32");

    /* Cross check with manifest field value. Following must be
     * supported if direct request is supported */
    if (messaging_type & FFA_DIRECT_REQUEST_SEND)
    {
        if (status_2 && status_4)
        {
            LOG(ERROR, "\tInvalid return code for direct messaging ABIs\n", 0, 0)
            return VAL_ERROR_POINT(19);
        }
    }
    else
    {
        if (!status_2 || !status_4)
        {
            LOG(ERROR, "\tInvalid return code for direct messaging ABIs\n", 0, 0)
            return VAL_ERROR_POINT(20);
        }
    }

    /* Cross check with manifest field value. Following must be
     * supported if direct respond is supported */
    if (messaging_type & FFA_RECEIPT_DIRECT_REQUEST_SUPPORT)
    {
        if (status_3 && status_5)
        {
            LOG(ERROR, "\tInvalid return code for direct messaging ABIs\n", 0, 0)
            return VAL_ERROR_POINT(21);
        }
    }
    else
    {
        if (!status_3 || !status_5)
        {
            LOG(ERROR, "\tInvalid return code for direct messaging ABIs\n", 0, 0)
            return VAL_ERROR_POINT(22);
        }
    }

    /* Either of the messaging method must be supported */
    if ((status_1 == VAL_SKIP_CHECK) && (status_2 == VAL_SKIP_CHECK))
    {
        LOG(ERROR, "\tEither of the messaging method must be supported\n", 0, 0)
        return VAL_ERROR_POINT(23);
    }

    val_reprogram_watchdog();

    /* Check invalid FID */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_INVALID_FID;
    val_ffa_features(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_NOT_SUPPORTED))
    {
        return VAL_ERROR_POINT(24);
    }

    /* FFA_NORMAL_WORLD_RESUME_32 must be not supported at instances
     * other than secure physical instance
     * */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_NORMAL_WORLD_RESUME_32;
    val_ffa_features(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_NOT_SUPPORTED))
    {
        return VAL_ERROR_POINT(25);
    }

    /* Check output w4-w7 reserved(MBZ) */
    output_reserve_count = 4;
    if (val_reserve_param_check(payload, output_reserve_count))
        return VAL_ERROR_POINT(26);

    (void)test_run_data;
    return VAL_SUCCESS;
}
