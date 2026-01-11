/*
 * Copyright (c) 2021-2026, Arm Limited or its affiliates. All rights reserved.
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

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = fid;
#if (PLATFORM_FFA_V >= FFA_V_1_1)
    if (fid == FFA_MEM_RETRIEVE_REQ_32 || fid == FFA_MEM_RETRIEVE_REQ_64)
    {
        payload.arg2 = 0x2;
    }
#endif
    LOG(DBG, "FFA_FEATURE Query idx %s fid %x\n", str, fid);
    val_ffa_features(&payload);

    if (payload.fid == FFA_ERROR_32 && (payload.arg2 == FFA_ERROR_NOT_SUPPORTED))
    {
        LOG(DBG, "%s -> feature not supported\n", str);
    }
    else if (payload.fid == FFA_SUCCESS_32 || payload.fid == FFA_SUCCESS_64)
    {
        LOG(DBG, "%s -> feature supported\n", str);
    }
    else
    {
        LOG(ERROR, "%s -Invalid return code received, fid=%x, err=%x\n",
            str, payload.fid, payload.arg2);
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
        case FFA_RXTX_UNMAP_32:
            if (payload.fid == FFA_ERROR_32)
            {
                LOG(ERROR, "fid = 0x%x must be supported\n", fid);
                status = VAL_ERROR_POINT(2);
                break;
            }
            /* Check output w2-w7 reserved(MBZ) */
            output_reserve_count = 6;
            if (val_reserve_param_check(payload, output_reserve_count))
            {
                LOG(ERROR, "reserved registers must be zero\n");
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
                LOG(ERROR, "reserved registers must be zero\n");
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
                LOG(DBG, "RXTX_MAP alignment boundary is 4K size\n");
            }
            else if (data == FFA_RXTX_MAP_64K_SIZE)
            {
                LOG(DBG, "RXTX_MAP alignment boundary is 64K size\n");
            }
            else if (data == FFA_RXTX_MAP_16K_SIZE)
            {
                LOG(DBG, "RXTX_MAP alignment boundary is 16K size\n");
            }
            else
            {
                LOG(ERROR, "FFA_RXTX_MAP_64 alignment boundary not defined\n");
                status = VAL_ERROR_POINT(5);
                break;
            }

#if PLATFORM_FFA_V < FFA_V_1_2
            /* Output w2[31:2] and w3 are reserved(MBZ) */
            data = VAL_EXTRACT_BITS(payload.arg2, 2, 31);
            if (data)
            {
                LOG(ERROR, "w2[31:2] must be zero\n");
                status = VAL_ERROR_POINT(6);
                break;
            }
#else
            /* Output w2[15:2] and w3 are reserved(MBZ) */
            data = VAL_EXTRACT_BITS(payload.arg2, 2, 15);
            if (data)
            {
                LOG(ERROR, "w2[15:2] must be zero\n");
                status = VAL_ERROR_POINT(6);
                break;
            }
            /* Check RXTX_MAP Maximum buffer size w2[31:16] no of pages,
               zero size means no limit specified */
            data = VAL_EXTRACT_BITS(payload.arg2, 16, 31);
            LOG(DBG, "RXTX_MAP Maximum buffer size w2[31:16] %x\n", data);
#endif
            /* Check output w3-w7 reserved(MBZ) */
            output_reserve_count = 5;
            if (val_reserve_param_check(payload, output_reserve_count))
            {
                LOG(ERROR, "reserved registers must be zero\n");
                status = VAL_ERROR_POINT(7);
            }
            break;

        /* FF-A optional features */
        case FFA_MSG_POLL_32:
        case FFA_RUN_32:
        case FFA_MSG_WAIT_32:
        case FFA_YIELD_32:
        case FFA_MSG_SEND_32:
        case FFA_MSG_SEND2_32:
        case FFA_MSG_SEND_DIRECT_REQ_32:
        case FFA_MSG_SEND_DIRECT_RESP_32:
        case FFA_MSG_SEND_DIRECT_REQ_64:
        case FFA_MSG_SEND_DIRECT_RESP_64:
        case FFA_MSG_SEND_DIRECT_REQ2_64:
        case FFA_MSG_SEND_DIRECT_RESP2_64:
        case FFA_MEM_RETRIEVE_RESP_32:
        case FFA_MEM_RELINQUISH_32:
        case FFA_MEM_RECLAIM_32:
        case FFA_PARTITION_INFO_GET_REGS_64:
        case FFA_SPM_ID_GET_32:
        case FFA_CONSOLE_LOG_32:
        case FFA_CONSOLE_LOG_64:
        case FFA_MEM_PERM_GET_64:
        case FFA_MEM_PERM_SET_64:
        case FFA_MEM_PERM_GET_32:
        case FFA_MEM_PERM_SET_32:
        case FFA_NOTIFICATION_BIND:
        case FFA_NOTIFICATION_UNBIND:
        case FFA_NOTIFICATION_SET:
        case FFA_NOTIFICATION_GET:
        case FFA_NOTIFICATION_INFO_GET_32:
        case FFA_NOTIFICATION_INFO_GET_64:
        case FFA_NOTIFICATION_BITMAP_CREATE:
        case FFA_NOTIFICATION_BITMAP_DESTROY:

            if (payload.fid == FFA_ERROR_32)
            {
                status = VAL_SKIP_CHECK;
                break;
            }
            /* Check output w2-w7 reserved(MBZ) */
            output_reserve_count = 6;
            if (val_reserve_param_check(payload, output_reserve_count))
            {
                LOG(ERROR, "reserved registers must be zero\n");
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
#if (PLATFORM_FFA_V <= FFA_V_1_1)
            /* Check for Dynamically allocated buffer support w2[0] */
            data = VAL_EXTRACT_BITS(payload.arg2, 0, 0);
            LOG(DBG, str);
            if (data == FFA_DYNAMIC_BUFFER_SUPPORT)
            {
                LOG(DBG, "Dynamic buffer supported\n");
            }
            else
            {
                LOG(DBG, "Dynamic buffer not supported\n");
            }

            /* Output w2[31:1] and w3 are reserved(MBZ) */
            data = VAL_EXTRACT_BITS(payload.arg2, 1, 31);
            if (data || payload.arg3)
                status = VAL_ERROR_POINT(9);
            break;
#else
            /* Check output w2-w7 reserved(MBZ) */
            output_reserve_count = 6;
            if (val_reserve_param_check(payload, output_reserve_count))
            {
                LOG(ERROR, "reserved registers must be zero\n");
                status = VAL_ERROR_POINT(9);
            }

            break;
#endif
        case FFA_MEM_RETRIEVE_REQ_64:
        case FFA_MEM_RETRIEVE_REQ_32:
            if (payload.fid == FFA_ERROR_32)
            {
                status = VAL_SKIP_CHECK;
                break;
            }
            /* Check for Dynamically allocated buffer support w2[0] */
            data = VAL_EXTRACT_BITS(payload.arg2, 0, 0);
            LOG(DBG, str);
            if (data == FFA_DYNAMIC_BUFFER_SUPPORT)
            {
                LOG(DBG, "Dynamic buffer supported\n");
            }
            else
            {
                LOG(DBG, "Dynamic buffer not supported\n");
            }

            /* Check for Outstanding retrievals field w3[7:0] */
            data = VAL_EXTRACT_BITS(payload.arg3, 0, 7);
            LOG(DBG, str);
            LOG(DBG, "Outstanding retrievals count %d\n", data);

            /* Output w2[31:1] and w3[31:8] are reserved(MBZ) */
#if (PLATFORM_FFA_V == FFA_V_1_0)
            data = VAL_EXTRACT_BITS(payload.arg2, 1, 31);
#else
            data = VAL_EXTRACT_BITS(payload.arg2, 3, 31);
#endif
            data1 = VAL_EXTRACT_BITS(payload.arg3, 8, 31);
            if (data || data1)
                status = VAL_ERROR_POINT(10);
            break;

        default:
            LOG(INFO, "fid=%x invalid feature \n", fid);
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
    uint32_t final_test_status = 0;
    uint32_t memory_management_support = 1;
    uint32_t messaging_type, notification_support;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    val_endpoint_info_t *ep_info;

    ep_info = val_get_endpoint_info();
    if (!ep_info)
    {
        LOG(ERROR, "get_endpoint_info error!\n");
        return VAL_ERROR_POINT(11);
    }

    messaging_type = ep_info[client_logical_id].ep_properties;
    notification_support = ep_info[client_logical_id].ep_properties;

    status_1 = ffa_feature_query(FFA_ERROR_32, "FFA_ERROR_32");

    if (status_1 == VAL_ERROR_POINT(2))
        final_test_status = 1;
    else if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_SUCCESS_32, "FFA_SUCCESS_32");
    status_2 = ffa_feature_query(FFA_SUCCESS_64, "FFA_SUCCESS_64");

    if ((status_1 != 0 && status_1 != VAL_SKIP_CHECK) &&
        (status_2 != 0 && status_2 != VAL_SKIP_CHECK))
        return VAL_ERROR_POINT(12);

    if (status_1 == VAL_SKIP_CHECK && status_2 == VAL_SKIP_CHECK) {
        LOG(WARN, "FFA_SUCCESS_32 or FFA_SUCCESS_64 atleast one must be supported\n");
        final_test_status = 1;
    }

    status_1 = ffa_feature_query(FFA_INTERRUPT_32, "FFA_INTERRUPT_32");
    if (status_1 == VAL_ERROR_POINT(2))
        final_test_status = 1;
    else if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_VERSION_32, "FFA_VERSION_32");
    if (status_1 == VAL_ERROR_POINT(2))
        final_test_status = 1;
    else if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_FEATURES_32, "FFA_FEATURES_32");
    if (status_1 == VAL_ERROR_POINT(2))
        final_test_status = 1;
    else if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_RX_RELEASE_32, "FFA_RX_RELEASE_32");
    if (status_1 == VAL_ERROR_POINT(2))
        final_test_status = 1;
    else if (status_1)
        return status_1;

    val_reprogram_watchdog();

    status_1 = ffa_feature_query(FFA_RXTX_UNMAP_32, "FFA_RXTX_UNMAP_32");
    if (status_1 == VAL_ERROR_POINT(2))
        final_test_status = 1;
    else if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_PARTITION_INFO_GET_32, "FFA_PARTITION_INFO_GET_32");
    if (status_1 == VAL_ERROR_POINT(2))
        final_test_status = 1;
    else if (status_1)
        return status_1;

    status_1 = ffa_feature_query(FFA_ID_GET_32, "FFA_ID_GET_32");
    if (status_1 == VAL_ERROR_POINT(2))
        final_test_status = 1;
    else if (status_1)
        return status_1;


    status_1 = ffa_feature_query(FFA_MSG_WAIT_32, "FFA_MSG_WAIT_32");
    if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
        return status_1;


    status_1 = ffa_feature_query(FFA_RUN_32, "FFA_RUN_32");
    if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
        return status_1;


    status_1 = ffa_feature_query(FFA_RXTX_MAP_32, "FFA_RXTX_MAP_32");
    status_2 = ffa_feature_query(FFA_RXTX_MAP_64, "FFA_RXTX_MAP_64");
    if ((status_1 != 0 && status_1 != VAL_SKIP_CHECK) &&
        (status_2 != 0 && status_2 != VAL_SKIP_CHECK))
        return VAL_ERROR_POINT(13);

    if (status_1 == VAL_SKIP_CHECK && status_2 == VAL_SKIP_CHECK) {
        LOG(WARN, "FFA_RXTX_MAP_32 or FFA_RXTX_MAP_64 atleast one must be supported\n");
        final_test_status = 1;
    }

    val_reprogram_watchdog();

    status_1 = ffa_feature_query(FFA_MEM_SHARE_64, "FFA_MEM_SHARE_64");
    status_2 = ffa_feature_query(FFA_MEM_SHARE_32, "FFA_MEM_SHARE_32");
    status_3 = ffa_feature_query(FFA_MEM_LEND_64, "FFA_MEM_LEND_64");
    status_4 = ffa_feature_query(FFA_MEM_LEND_32, "FFA_MEM_LEND_32");
    status_5 = ffa_feature_query(FFA_MEM_DONATE_64, "FFA_MEM_DONATE_64");
    status_6 = ffa_feature_query(FFA_MEM_DONATE_32, "FFA_MEM_DONATE_32");

    /* At least one of the ABI must be supported - share/donate/lend */
    if (status_1 && status_2 && status_3
         && status_4 && status_5 && status_6)
    {
           LOG(WARN, "No memory manage ABI was supported\n");
           memory_management_support = 0;
    }

    if ((status_1 != 0 && status_1 != VAL_SKIP_CHECK) &&
        (status_2 != 0 && status_2 != VAL_SKIP_CHECK) &&
        (status_3 != 0 && status_3 != VAL_SKIP_CHECK) &&
        (status_4 != 0 && status_4 != VAL_SKIP_CHECK) &&
        (status_5 != 0 && status_5 != VAL_SKIP_CHECK) &&
        (status_6 != 0 && status_6 != VAL_SKIP_CHECK)) {
            return VAL_ERROR_POINT(14);
    }

    val_reprogram_watchdog();

    if (memory_management_support == 1) {

        status_7 = ffa_feature_query(FFA_MEM_RETRIEVE_REQ_64, "FFA_MEM_RETRIEVE_REQ_64");
        status_8 = ffa_feature_query(FFA_MEM_RETRIEVE_REQ_32, "FFA_MEM_RETRIEVE_REQ_32");
        status_9 = ffa_feature_query(FFA_MEM_RETRIEVE_RESP_32, "FFA_MEM_RETRIEVE_RESP_32");
        status_10 = ffa_feature_query(FFA_MEM_RELINQUISH_32, "FFA_MEM_RELINQUISH_32");
        status_11 = ffa_feature_query(FFA_MEM_RECLAIM_32, "FFA_MEM_RECLAIM_32");

        (void) status_7;
        (void) status_8;
        (void) status_9;
        (void) status_10;
        (void) status_11;

#if !defined(XEN_SUPPORT) && !(TARGET_LINUX_ENV == 1)
        /* If 64 bit manage ABI supported */
        if (!status_1 || !status_3 || !status_5)
        {
            if (status_7 || status_9 || status_10 || status_11) {
               LOG(WARN, "RETRIEVE/RELINQUISH/RECLAIM all must be supported\n");
               final_test_status = 1;
            }
        }

        /* If 32 bit manage ABI supported */
        if (!status_2 || !status_4 || !status_6)
        {
            if (status_8 || status_9 || status_10 || status_11) {
               LOG(WARN, "RETRIEVE/RELINQUISH/RECLAIM all must be supported\n");
               final_test_status = 1;
            }

        }
#endif
    }
     val_reprogram_watchdog();

#if PLATFORM_FFA_V == FFA_V_1_0
    status_1 = ffa_feature_query(FFA_MSG_SEND_32, "FFA_MSG_SEND_32");
    status_2 = ffa_feature_query(FFA_MSG_POLL_32, "FFA_MSG_POLL_32");
    /* Cross check with manifest field value. Following must be
     * supported if indirect messaging is supported */
    if ((messaging_type & FFA_INDIRECT_MESSAGE_SUPPORT) &&
            (val_get_curr_endpoint_id() != HYPERVISOR_ID))
    {
        if (status_1 || status_2)
        {
            LOG(ERROR, "Invalid return code for indirect messaging ABIs\n");
            final_test_status = 1;
        }
    }
    else
    {
        if (!status_1 || !status_2)
        {
            LOG(ERROR, "Invalid return code for indirect messaging ABIs\n");
            final_test_status = 1;
        }
    }
#else
    /* Check Only if Indirect Messaging is Supported by the End Point */
    if ((messaging_type & FFA_INDIRECT_MESSAGE_SUPPORT))
    {
        status_1 = ffa_feature_query(FFA_MSG_SEND2_32, "FFA_MSG_SEND2_32");
        if (status_1)
        {
            LOG(ERROR, "Invalid return code for indirect messaging ABIs\n");
            final_test_status = 1;

        }
    }

#endif
    status_2 = ffa_feature_query(FFA_YIELD_32, "FFA_YIELD_32");
    if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
        return status_1;


    val_reprogram_watchdog();

    status_2 = ffa_feature_query(FFA_MSG_SEND_DIRECT_REQ_64, "FFA_MSG_SEND_DIRECT_REQ_64");
    status_3 = ffa_feature_query(FFA_MSG_SEND_DIRECT_RESP_64, "FFA_MSG_SEND_DIRECT_RESP_64");
    status_4 = ffa_feature_query(FFA_MSG_SEND_DIRECT_REQ_32, "FFA_MSG_SEND_DIRECT_REQ_32");
    status_5 = ffa_feature_query(FFA_MSG_SEND_DIRECT_RESP_32, "FFA_MSG_SEND_DIRECT_RESP_32");

    /* Cross check with manifest field value. Following must be
     * supported if direct request is supported */
    if (messaging_type & FFA_DIRECT_REQUEST_SEND)
    {
        if (status_2 && status_4)
        {
            LOG(ERROR, "Invalid return code for direct messaging ABIs\n");
            final_test_status = 1;
        }
    }
    else
    {
        if (!status_2 || !status_4)
        {
            LOG(ERROR, "Invalid return code for direct messaging ABIs\n");
            final_test_status = 1;
        }
    }


#if !defined(XEN_SUPPORT) && !(TARGET_LINUX_ENV == 1)
    /* Cross check with manifest field value. Following must be
     * supported if direct respond is supported */
    if (messaging_type & FFA_RECEIPT_DIRECT_REQUEST_SUPPORT)
    {
        if (status_3 && status_5)
        {
            LOG(ERROR, "Invalid return code for direct messaging ABIs\n");
            final_test_status = 1;
        }
    }
    else
    {
        if (!status_3 || !status_5)
        {
            LOG(ERROR, "Invalid return code for direct messaging ABIs\n");
            final_test_status = 1;
        }
    }

#endif



#if PLATFORM_FFA_V >= FFA_V_1_2

    status_6 = ffa_feature_query(FFA_MSG_SEND_DIRECT_REQ2_64, "FFA_MSG_SEND_DIRECT_REQ2_64");
    status_7 = ffa_feature_query(FFA_MSG_SEND_DIRECT_RESP2_64, "FFA_MSG_SEND_DIRECT_RESP2_64");

#if !defined(XEN_SUPPORT) && !(TARGET_LINUX_ENV == 1)
    /* Cross check with manifest field value. Following must be
     * supported if direct request is supported */
    if (messaging_type & FFA_DIRECT_REQUEST2_SEND)
    {
        if (status_6)
        {
            LOG(ERROR, "Invalid return code for direct messaging 2 ABIs\n");
            final_test_status = 1;
        }
    }
    else
    {
        if (!status_6)
        {
            LOG(ERROR, "Invalid return code for direct messaging 2 ABIs\n");
            final_test_status = 1;
        }
    }

    if (messaging_type & FFA_RECEIPT_DIRECT_REQUEST2_SUPPORT)
    {
        if (status_7)
        {
            LOG(ERROR, "Invalid return code for direct messaging 2 ABIs\n");
            final_test_status = 1;
        }
    }
    else
    {
        if (!status_7)
        {
            LOG(ERROR, "Invalid return code for direct messaging 2 ABIs\n");
            final_test_status = 1;
        }
    }
#endif
#endif

#if PLATFORM_FFA_V >= FFA_V_1_2
    if ((status_1 == VAL_SKIP_CHECK) && (status_2 == VAL_SKIP_CHECK) &&
        (status_4 == VAL_SKIP_CHECK) && (status_6 == VAL_SKIP_CHECK))
#else
    if ((status_1 == VAL_SKIP_CHECK) && (status_2 == VAL_SKIP_CHECK) &&
       (status_4 == VAL_SKIP_CHECK))
#endif
    {
        LOG(ERROR, "Either of the messaging method must be supported\n");
        final_test_status = 1;
    }

#if 0 // TODO: XEN Support Missing
#endif

    val_reprogram_watchdog();

#if PLATFORM_FFA_V >= FFA_V_1_2
    status_1 = ffa_feature_query(FFA_PARTITION_INFO_GET_REGS_64, "FFA_PARTITION_INFO_GET_REGS_64");
    if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
        return status_1;

    if (VAL_IS_ENDPOINT_SECURE(client_logical_id))
    {
        status_1 = ffa_feature_query(FFA_CONSOLE_LOG_32, "FFA_CONSOLE_LOG_32");
        if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
            return status_1;

        status_1 = ffa_feature_query(FFA_CONSOLE_LOG_64, "FFA_CONSOLE_LOG_64");
        if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
            return status_1;
    }
#endif

#if PLATFORM_FFA_V >= FFA_V_1_1
    status_1 = ffa_feature_query(FFA_SPM_ID_GET_32, "FFA_SPM_ID_GET_32");
    if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
        return status_1;

    if (VAL_IS_ENDPOINT_SECURE(client_logical_id))
    {
        status_1 = ffa_feature_query(FFA_MEM_PERM_SET_32, "FFA_MEM_PERM_SET_32");
        if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
            return status_1;

        status_1 = ffa_feature_query(FFA_MEM_PERM_GET_32, "FFA_MEM_PERM_GET_32");
        if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
            return status_1;

        status_1 = ffa_feature_query(FFA_MEM_PERM_SET_64, "FFA_MEM_PERM_SET_64");
        if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
            return status_1;

        status_1 = ffa_feature_query(FFA_MEM_PERM_GET_64, "FFA_MEM_PERM_GET_64");
        if (status_1 != VAL_SKIP_CHECK && status_1 != 0)
            return status_1;
    }
#endif
    val_reprogram_watchdog();

#if PLATFORM_FFA_V >= FFA_V_1_1
    /* Notification ABI */

    status_1 = ffa_feature_query(FFA_NOTIFICATION_BIND, "FFA_NOTIFICATION_BIND");
    status_2 = ffa_feature_query(FFA_NOTIFICATION_UNBIND, "FFA_NOTIFICATION_UNBIND");
    status_3 = ffa_feature_query(FFA_NOTIFICATION_SET, "FFA_NOTIFICATION_SET");
    status_4 = ffa_feature_query(FFA_NOTIFICATION_GET, "FFA_NOTIFICATION_GET");

    if ((status_1 != 0 && status_1 != VAL_SKIP_CHECK) &&
        (status_2 != 0 && status_2 != VAL_SKIP_CHECK) &&
        (status_3 != 0 && status_3 != VAL_SKIP_CHECK) &&
        (status_4 != 0 && status_4 != VAL_SKIP_CHECK))
        return VAL_ERROR_POINT(15);

    if (!(VAL_IS_ENDPOINT_SECURE(client_logical_id)))
    {
        status_5 = ffa_feature_query(FFA_NOTIFICATION_INFO_GET_32, "FFA_NOTIFICATION_INFO_GET_32");
        status_6 = ffa_feature_query(FFA_NOTIFICATION_INFO_GET_64, "FFA_NOTIFICATION_INFO_GET_64");
        if ((status_5 != 0 && status_5 != VAL_SKIP_CHECK) &&
            (status_6 != 0 && status_6 != VAL_SKIP_CHECK))
            return VAL_ERROR_POINT(16);
    }

#if (PLATFORM_NS_HYPERVISOR_PRESENT == 0)
        status_7 = ffa_feature_query(FFA_NOTIFICATION_BITMAP_CREATE,
                                     "FFA_NOTIFICATION_BITMAP_CREATE");
        status_8 = ffa_feature_query(FFA_NOTIFICATION_BITMAP_DESTROY,
                                     "FFA_NOTIFICATION_BITMAP_DESTROY");

        if ((status_7 != 0 && status_7 != VAL_SKIP_CHECK) &&
            (status_8 != 0 && status_8 != VAL_SKIP_CHECK))
            return VAL_ERROR_POINT(17);
#endif
#if !defined(XEN_SUPPORT) && !(TARGET_LINUX_ENV == 1)
    /* Cross check with manifest field value. Following must be
     * supported if notification is supported */
    if (notification_support & FFA_NOTIFICATION_SUPPORT)
    {
        if (status_3)
        {
            LOG(ERROR, "Invalid return code for Notification ABIs\n");
            final_test_status = 1;
        }
    }
    else
    {
        if (!status_3)
        {
            LOG(ERROR, "Invalid return code for Notification ABIs\n");
            final_test_status = 1;
        }
    }
#endif
#endif

    val_reprogram_watchdog();
    LOG(DBG, "VAL Watchdog Reprogram Complete\n");

    /* Check invalid FID */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_INVALID_FID;
    val_ffa_features(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_NOT_SUPPORTED))
    {
        final_test_status = 1;
    }

    /* FFA_NORMAL_WORLD_RESUME_32 must be not supported at instances
     * other than secure physical instance
     * */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_NORMAL_WORLD_RESUME_32;
    val_ffa_features(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_NOT_SUPPORTED))
    {
        LOG(ERROR, "FFA_NORMAL_WORLD_RESUME ABI should not be supported\n");
        final_test_status = 1;
    }
    /* Check output w4-w7 reserved(MBZ) */
    output_reserve_count = 4;
    if (val_reserve_param_check(payload, output_reserve_count))
        return VAL_ERROR_POINT(18);


#if PLATFORM_FFA_V >= FFA_V_1_1
    /* FFA_RX_ACQUIRE_32 must be not supported at instances
     * other than secure and non-secure physical instance
     */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_RX_ACQUIRE_32;
    val_ffa_features(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_NOT_SUPPORTED))
    {
        LOG(ERROR, "FFA_RX_ACQUIRE_32 should not be supported\n");
        final_test_status = 1;
    }
    /* Check output w4-w7 reserved(MBZ) */
    output_reserve_count = 4;
    if (val_reserve_param_check(payload, output_reserve_count))
        return VAL_ERROR_POINT(19);

#endif
    (void)test_run_data;
    if (final_test_status)
        return VAL_ERROR_POINT(20);

    return VAL_SUCCESS;
}
