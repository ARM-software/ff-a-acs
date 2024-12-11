/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_interfaces.h"
#include "val_ffa.h"

static ffa_args_t ffa_smccc(uint64_t fid, uint64_t arg1, uint64_t arg2,
                            uint64_t arg3, uint64_t arg4, uint64_t arg5,
                            uint64_t arg6, uint64_t arg7, uint64_t arg8,
                            uint64_t arg9, uint64_t arg10, uint64_t arg11,
                            uint64_t arg12, uint64_t arg13, uint64_t arg14,
                            uint64_t arg15, uint64_t arg16, uint64_t arg17)
{
    ffa_args_t args;

    args.fid = fid;
    args.arg1 = arg1;
    args.arg2 = arg2;
    args.arg3 = arg3;
    args.arg4 = arg4;
    args.arg5 = arg5;
    args.arg6 = arg6;
    args.arg7 = arg7;
    args.ext_args.arg8 = arg8;
    args.ext_args.arg9 = arg9;
    args.ext_args.arg10 = arg10;
    args.ext_args.arg11 = arg11;
    args.ext_args.arg12 = arg12;
    args.ext_args.arg13 = arg13;
    args.ext_args.arg14 = arg14;
    args.ext_args.arg15 = arg15;
    args.ext_args.arg16 = arg16;
    args.ext_args.arg17 = arg17;
    val_call_conduit(&args);
    return args;
}

static void ffa_error(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_ERROR_32, args->arg1, args->arg2, args->arg3,
                      args->arg4, args->arg5, args->arg6, args->arg7,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief Returns error code in response to a previous invocation
          of a FF-A function.
 * @param args - Input arguments to FFA_ERROR abi.
 * @return Error code in response to FF-A function.
**/
void val_ffa_error(ffa_args_t *args)
{
    ffa_error(args);
}

static void ffa_success(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_SUCCESS_64, args->arg1, args->arg2, args->arg3,
                          args->arg4, args->arg5, args->arg6, args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_SUCCESS_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

/**
 * @brief Returns results upon successful completion of a previous
          invocation of a FF-A function.
 * @param args - Input arguments to FFA_SUCCESS_32 abi.
 * @return Returns success status code in response to FF-A function.
**/
void val_ffa_success_32(ffa_args_t *args)
{
    ffa_success(args, false);
}

/**
 * @brief Returns results upon successful completion of a previous
          invocation of a FF-A function.
 * @param args - Input arguments to FFA_SUCCESS_64 abi.
 * @return Returns success status code in response to FF-A function.
**/
void val_ffa_success_64(ffa_args_t *args)
{
    ffa_success(args, true);
}

static void ffa_version(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_VERSION_32, (uint32_t)args->arg1,
                      (uint32_t)args->arg2, (uint32_t)args->arg3,
                      (uint32_t)args->arg4, (uint32_t)args->arg5,
                      (uint32_t)args->arg6, (uint32_t)args->arg7,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief Returns version of the Firmware Framework implementation
 *        at a FF-A instance.
 * @param args - Input arguments to FFA_VERSION abi.
 * @return Returns FFA version.
**/
void val_ffa_version(ffa_args_t *args)
{
    ffa_version(args);
}

static void ffa_msg_send_direct_req2(ffa_args_t *args)
{
#ifdef TARGET_LINUX
    ffa_args_t  payload;
#endif

    *args = ffa_smccc(FFA_MSG_SEND_DIRECT_REQ2_64, args->arg1, args->arg2,
                        args->arg3, args->arg4, args->arg5, args->arg6,
                        args->arg7, args->ext_args.arg8, args->ext_args.arg9, args->ext_args.arg10,
                        args->ext_args.arg11, args->ext_args.arg12, args->ext_args.arg13,
                        args->ext_args.arg14, args->ext_args.arg15, args->ext_args.arg16,
                        args->ext_args.arg17);

#ifdef TARGET_LINUX
    while (args->fid == FFA_INTERRUPT_32)
    {
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = args->arg1;
        val_ffa_run(&payload);
        *args = payload;
    }
#endif
}

/**
 * @brief - Send a Partition message in parameter registers as a request to
 *          a target endpoint, run the endpoint and block until a response is
 *          available
 * @param args - Input arguments to FFA_MSG_SEND_DIRECT_REQ2_64 abi.
 * @return - Returns success/error status code in response to
 *           FFA_MSG_SEND_DIRECT_REQ function.
**/
void val_ffa_msg_send_direct_req2_64(ffa_args_t *args)
{
    ffa_msg_send_direct_req2(args);
}

static void ffa_msg_send_direct_req(ffa_args_t *args, bool arch64)
{
#ifdef TARGET_LINUX
    ffa_args_t  payload;
#endif

    if (arch64)
    {
        *args = ffa_smccc(FFA_MSG_SEND_DIRECT_REQ_64, args->arg1, args->arg2,
                          args->arg3, args->arg4, args->arg5, args->arg6,
                          args->arg7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_MSG_SEND_DIRECT_REQ_32, args->arg1, args->arg2,
                          args->arg3, args->arg4, args->arg5, args->arg6,
                          args->arg7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

#ifdef TARGET_LINUX
    while (args->fid == FFA_INTERRUPT_32)
    {
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = args->arg1;
        val_ffa_run(&payload);
        *args = payload;
    }
#endif
}

/**
 * @brief - Send a Partition message in parameter registers as a request to
 *          a target endpoint, run the endpoint and block until a response is
 *          available
 * @param args - Input arguments to FFA_MSG_SEND_DIRECT_REQ_32 abi.
 * @return - Returns success/error status code in response to
 *           FFA_MSG_SEND_DIRECT_REQ function.
**/
void val_ffa_msg_send_direct_req_32(ffa_args_t *args)
{
    ffa_msg_send_direct_req(args, false);
}

/**
 * @brief - Send a Partition message in parameter registers as a request to
 *          a target endpoint, run the endpoint and block until a response is
 *          available
 * @param args - Input arguments to FFA_MSG_SEND_DIRECT_REQ_64 abi.
 * @return - Returns success/error status code in response to
 *           FFA_MSG_SEND_DIRECT_REQ function.
**/
void val_ffa_msg_send_direct_req_64(ffa_args_t *args)
{
    ffa_msg_send_direct_req(args, true);
}

static void ffa_msg_send_direct_resp(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_MSG_SEND_DIRECT_RESP_64, args->arg1, args->arg2,
                          args->arg3, args->arg4, args->arg5, args->arg6,
                          args->arg7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_MSG_SEND_DIRECT_RESP_32, args->arg1, args->arg2,
                          args->arg3, args->arg4, args->arg5, args->arg6,
                          args->arg7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}


static void ffa_msg_send_direct_resp2(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_MSG_SEND_DIRECT_RESP2_64, args->arg1, args->arg2,
                        args->arg3, args->arg4, args->arg5, args->arg6,
                        args->arg7, args->ext_args.arg8, args->ext_args.arg9, args->ext_args.arg10,
                        args->ext_args.arg11, args->ext_args.arg12, args->ext_args.arg13,
                        args->ext_args.arg14, args->ext_args.arg15, args->ext_args.arg16,
                        args->ext_args.arg17);
}

/**
 * @brief - Send a Partition message in parameter registers as a response to
 *          a target endpoint, run the endpoint and block until a response is
 *          available
 * @param args - Input arguments to FFA_MSG_SEND_DIRECT_RESP_64 abi.
 * @return - Returns success/error status code in response to
 *           FFA_MSG_SEND_DIRECT_RESP function.
**/
void val_ffa_msg_send_direct_resp2_64(ffa_args_t *args)
{
    ffa_msg_send_direct_resp2(args);
}

/**
 * @brief - Send a Partition message in parameter registers as a response to
 *          a target endpoint, run the endpoint and block until a response is
 *          available
 * @param args - Input arguments to FFA_MSG_SEND_DIRECT_RESP_32 abi.
 * @return - Returns success/error status code in response to
 *           FFA_MSG_SEND_DIRECT_RESP function.
**/
void val_ffa_msg_send_direct_resp_32(ffa_args_t *args)
{
    ffa_msg_send_direct_resp(args, false);
}

/**
 * @brief - Send a Partition message in parameter registers as a response to
 *          a target endpoint, run the endpoint and block until a response is
 *          available
 * @param args - Input arguments to FFA_MSG_SEND_DIRECT_RESP_64 abi.
 * @return - Returns success/error status code in response to
 *           FFA_MSG_SEND_DIRECT_RESP function.
**/
void val_ffa_msg_send_direct_resp_64(ffa_args_t *args)
{
    ffa_msg_send_direct_resp(args, true);
}

static void ffa_id_get(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_ID_GET_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void ffa_spm_id_get(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_SPM_ID_GET_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Returns 16-bit ID of calling FF-A component.
 * @param id - Store the 16-bit ID.
 * @return - status code.
**/
ffa_endpoint_id_t val_get_curr_endpoint_id(void)
{
    ffa_args_t ret;
    ffa_endpoint_id_t id;

    ret = ffa_smccc(FFA_ID_GET_32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (ret.fid == FFA_ERROR_32)
        VAL_PANIC("Error: FFA_ID_GET_32 failed");

    id = ret.arg2 & 0xffff;
    return id;
}

/**
 * @brief - Returns 16-bit ID of calling FF-A component.
 * @param args - Input arguments to FFA_SPM_ID_GET_32 abi.
 * @return - void.
**/
void val_ffa_id_get(ffa_args_t *args)
{
    ffa_id_get(args);
}

/**
 * @brief - Returns 16-bit ID of the SPMC or SPMD.
 * @param args - Input arguments to FFA_SPM_ID_GET_32 abi.
 * @return - void.
**/
void val_ffa_spm_id_get(ffa_args_t *args)
{
    ffa_spm_id_get(args);
}

static void ffa_rx_release(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_RX_RELEASE_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Relinquish ownership of a RX buffer after reading a message from it.
 * @param args - Input arguments to FFA_RX_RELEASE abi.
 * @return - void.
**/
void val_ffa_rx_release(ffa_args_t *args)
{
    ffa_rx_release(args);
}

static void ffa_rxtx_unmap(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_RXTX_UNMAP_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

}
/**
 * @brief - Unmaps an endpoint's RX/TX buffer pair from the callee's
            translation regime.
 * @param args - Input arguments to FFA_RXTX_UNMAP abi.
 * @return - void.
**/
void val_ffa_rxtx_unmap(ffa_args_t *args)
{
    ffa_rxtx_unmap(args);
}

static void ffa_rxtx_map(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_RXTX_MAP_64, args->arg1, args->arg2, args->arg3,
                                args->arg4, args->arg5, args->arg6, args->arg7,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_RXTX_MAP_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

/**
 * @brief - Maps the RX/TX buffer pair in the callee's translation regime
 *          on behalf of an endpoint.
 * @param args - Input arguments to FFA_RXTX_MAP_32 abi.
 * @return - Returns success/error status code in response to
 *           FFA_RXTX_MAP_32 function.
**/
void val_ffa_rxtx_map_32(ffa_args_t *args)
{
    ffa_rxtx_map(args, false);
}

/**
 * @brief - Maps the RX/TX buffer pair in the callee's translation regime
 *          on behalf of an endpoint.
 * @param args - Input arguments to FFA_RXTX_MAP_64 abi.
 * @return - Returns success/error status code in response to
 *           FFA_RXTX_MAP_64 function.
**/
void val_ffa_rxtx_map_64(ffa_args_t *args)
{
    ffa_rxtx_map(args, true);
}

static void ffa_msg_send(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_MSG_SEND_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Send a Partition message to a VM through the RX/TX buffers
 *          by using indirect messaging
 * @param args - Input arguments to FFA_MSG_SEND abi.
 * @return - Returns success/error status code in response to
 *           FFA_MSG_SEND function.
**/
void val_ffa_msg_send(ffa_args_t *args)
{
    ffa_msg_send(args);
}

static void ffa_msg_send2(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_MSG_SEND2_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Send a Partition message 2 to a VM through the RX/TX buffers
 *          by using indirect messaging
 * @param args - Input arguments to FFA_MSG_SEND2 abi.
 * @return - Returns success/error status code in response to
 *           FFA_MSG_SEND2 function.
**/
void val_ffa_msg_send2(ffa_args_t *args)
{
    ffa_msg_send2(args);
}

static void ffa_partition_info_get_regs(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_PARTITION_INFO_GET_REGS_64, args->arg1, args->arg2,
                        args->arg3, args->arg4, args->arg5, args->arg6,
                        args->arg7, args->ext_args.arg8, args->ext_args.arg9, args->ext_args.arg10,
                        args->ext_args.arg11, args->ext_args.arg12, args->ext_args.arg13,
                        args->ext_args.arg14, args->ext_args.arg15, args->ext_args.arg16,
                        args->ext_args.arg17);
}

/**
 * @brief - Request Hypervisor and SPM to return information about
 *          partitions instantiated in the system.
 * @param args - Input arguments to FFA_PARTITION_INFO_GET abi.
 * @return - Returns success/error status code in response to
 *           FFA_PARTITION_INFO_GET function.
**/
void val_ffa_partition_info_get_regs(ffa_args_t *args)
{
    ffa_partition_info_get_regs(args);
}

static void ffa_partition_info_get(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_PARTITION_INFO_GET_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Request Hypervisor and SPM to return information about
 *          partitions instantiated in the system.
 * @param args - Input arguments to FFA_PARTITION_INFO_GET abi.
 * @return - Returns success/error status code in response to
 *           FFA_PARTITION_INFO_GET function.
**/
void val_ffa_partition_info_get(ffa_args_t *args)
{
    ffa_partition_info_get(args);
}

static void ffa_features(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_FEATURES_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - This interface is used by a FF-A component at the lower EL
 *          at a FF-A instance to query.
 * @param args - Input arguments to FFA_FEATURES abi.
 * @return - Returns success/error status code or interface properties in
 *           response to FFA_FEATURES function.
**/
void val_ffa_features(ffa_args_t *args)
{
    ffa_features(args);
}

static void ffa_mem_reclaim(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_MEM_RECLAIM_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Restores exclusive access to a memory region back to its Owner.
 * @param args - Input arguments to FFA_MEM_RECLAIM abi.
 * @return - Returns success/error status code in
 *           response to FFA_MEM_RECLAIM function.
**/
void val_ffa_mem_reclaim(ffa_args_t *args)
{
    ffa_mem_reclaim(args);
}

static void ffa_msg_wait(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_MSG_WAIT_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Blocks the caller until message available in RX buffer
 *          or parameter registers
 * @param args - Input arguments to FFA_MSG_WAIT abi.
 * @return - Returns success/error status code in
 *           response to FFA_MSG_WAIT function.
**/
void val_ffa_msg_wait(ffa_args_t *args)
{
    ffa_msg_wait(args);
}

static void ffa_yield(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_YIELD_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Relinquish execution back to the scheduler on
 *          current physical CPU from the calling VM.
 * @param args - Input arguments to FFA_YIELD abi.
 * @return - Returns success/error status code in response to FFA_YIELD function.
**/
void val_ffa_yield(ffa_args_t *args)
{
    ffa_yield(args);
}

static void ffa_run(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_RUN_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Run an endpoint's execution context on the current PE.
 * @param args - Input arguments to FFA_RUN abi.
 * @return - Returns success/error status code in response to FFA_RUN abi.
**/
void val_ffa_run(ffa_args_t *args)
{
    ffa_run(args);
}

static void ffa_msg_poll(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_MSG_POLL_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Poll if a message is available in the caller's RX buffer.
 *          Execution is returned to the caller if no message is available.
 * @param args - Input arguments to FFA_MSG_POLL abi.
 * @return - Returns success/error status code in response to FFA_MSG_POLL abi.
**/
void val_ffa_msg_poll(ffa_args_t *args)
{
    ffa_msg_poll(args);
}

static void ffa_mem_donate(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_MEM_DONATE_64, args->arg1, args->arg2, args->arg3,
                                args->arg4, args->arg5, args->arg6, args->arg7,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_MEM_DONATE_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

/**
 * @brief - Starts a transaction to transfer of ownership of a memory region
 *          from a Sender endpoint to a Receiver endpoint.
 * @param args - Input arguments to FFA_MEM_DONATE_32 abi.
 * @return - Returns success/error status code in response to FFA_MEM_DONATE abi.
**/
void val_ffa_mem_donate_32(ffa_args_t *args)
{
    ffa_mem_donate(args, false);
}

/**
 * @brief - Starts a transaction to transfer of ownership of a memory region
 *          from a Sender endpoint to a Receiver endpoint.
 * @param args - Input arguments to FFA_MEM_DONATE_64 abi.
 * @return - Returns success/error status code in response to FFA_MEM_DONATE abi.
**/
void val_ffa_mem_donate_64(ffa_args_t *args)
{
    ffa_mem_donate(args, true);
}

static void ffa_mem_lend(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_MEM_LEND_64, args->arg1, args->arg2, args->arg3,
                                args->arg4, args->arg5, args->arg6, args->arg7,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_MEM_LEND_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

/**
 * @brief - Starts a transaction to transfer an Owner's access to a memory
 *          region and grant access to it to one or more Borrowers
 * @param args - Input arguments to FFA_MEM_LEND_32 abi.
 * @return - Returns success/error status code in response to FFA_MEM_LEND abi.
**/
void val_ffa_mem_lend_32(ffa_args_t *args)
{
    ffa_mem_lend(args, false);
}

/**
 * @brief - Starts a transaction to transfer an Owner's access to a memory
 *          region and grant access to it to one or more Borrowers.
 * @param args - Input arguments to FFA_MEM_LEND_64 abi.
 * @return - Returns success/error status code in response to FFA_MEM_LEND abi.
**/
void val_ffa_mem_lend_64(ffa_args_t *args)
{
    ffa_mem_lend(args, true);
}

static void ffa_mem_share(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_MEM_SHARE_64, args->arg1, args->arg2, args->arg3,
                                args->arg4, args->arg5, args->arg6, args->arg7,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_MEM_SHARE_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

/**
 * @brief - Starts a transaction to grant access to a memory region
 *          to one or more Borrowers.
 * @param args - Input arguments to FFA_MEM_SHARE_32 abi.
 * @return - Returns success/error status code in response to FFA_MEM_SHARE abi.
**/
void val_ffa_mem_share_32(ffa_args_t *args)
{
    ffa_mem_share(args, false);
}

/**
 * @brief - Starts a transaction to grant access to a memory region
 *          to one or more Borrowers.
 * @param args - Input arguments to FFA_MEM_SHARE_64 abi.
 * @return - Returns success/error status code in response to FFA_MEM_SHARE abi
**/
void val_ffa_mem_share_64(ffa_args_t *args)
{
    ffa_mem_share(args, true);
}

static void ffa_mem_retrieve(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_MEM_RETRIEVE_REQ_64, args->arg1, args->arg2,
                          args->arg3, args->arg4, args->arg5, args->arg6,
                          args->arg7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_MEM_RETRIEVE_REQ_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

/**
 * @brief - Requests completion of a donate, lend or share memory management
 *          transaction.
 * @param args - Input arguments to FFA_MEM_RETRIEVE_32 abi.
 * @return - Returns success/error status code in response to
            FFA_MEM_RETRIEVE_32 abi.
**/
void val_ffa_mem_retrieve_32(ffa_args_t *args)
{
    ffa_mem_retrieve(args, false);
}

/**
 * @brief - Requests completion of a donate, lend or share memory management
 *          transaction.
 * @param args - Input arguments to FFA_MEM_RETRIEVE_64 abi.
 * @return - Returns success/error status code in response to
            FFA_MEM_RETRIEVE_64 abi.
**/
void val_ffa_mem_retrieve_64(ffa_args_t *args)
{
    ffa_mem_retrieve(args, true);
}

static void ffa_mem_relinquish(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_MEM_RELINQUISH_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Starts a transaction to transfer access to a shared or lent memory
 *          region from a Borrower back to its Owner.
 * @param args - Input arguments to FFA_MEM_RELINQUISH abi.
 * @return - Returns success/error status code in response to
            FFA_MEM_RELINQUISH abi.
**/
void val_ffa_mem_relinquish(ffa_args_t *args)
{
    ffa_mem_relinquish(args);
}

static void ffa_secondary_ep_register_64(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_SECONDARY_EP_REGISTER_64,
                      (uint64_t)&pal_secondary_cpu_boot_entry,
                          args->arg2, args->arg3,
                          args->arg4, args->arg5,
                          args->arg6, args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/**
 * @brief - Register secondary cpu endpoint with SPMC
 * @param - Void
 * @return - Void
**/
void val_ffa_secondary_ep_register_64(void)
{
    ffa_args_t args;

    val_memset(&args, 0, sizeof(ffa_args_t));
    ffa_secondary_ep_register_64(&args);
}

void val_ffa_notification_bitmap_create(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_NOTIFICATION_BITMAP_CREATE, (uint32_t)args->arg1,
                                    (uint32_t)args->arg2, (uint32_t)args->arg3,
                                    (uint32_t)args->arg4, (uint32_t)args->arg5,
                                    (uint32_t)args->arg6, (uint32_t)args->arg7,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void val_ffa_notification_bitmap_destroy(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_NOTIFICATION_BITMAP_DESTROY, (uint32_t)args->arg1,
                                    (uint32_t)args->arg2, (uint32_t)args->arg3,
                                    (uint32_t)args->arg4, (uint32_t)args->arg5,
                                    (uint32_t)args->arg6, (uint32_t)args->arg7,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void val_ffa_notification_bind(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_NOTIFICATION_BIND, (uint32_t)args->arg1,
                                    (uint32_t)args->arg2, (uint32_t)args->arg3,
                                    (uint32_t)args->arg4, (uint32_t)args->arg5,
                                    (uint32_t)args->arg6, (uint32_t)args->arg7,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void val_ffa_notification_unbind(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_NOTIFICATION_UNBIND, (uint32_t)args->arg1,
                                    (uint32_t)args->arg2, (uint32_t)args->arg3,
                                    (uint32_t)args->arg4, (uint32_t)args->arg5,
                                    (uint32_t)args->arg6, (uint32_t)args->arg7,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void val_ffa_notification_set(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_NOTIFICATION_SET, (uint32_t)args->arg1,
                                    (uint32_t)args->arg2, (uint32_t)args->arg3,
                                    (uint32_t)args->arg4, (uint32_t)args->arg5,
                                    (uint32_t)args->arg6, (uint32_t)args->arg7,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void val_ffa_notification_get(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_NOTIFICATION_GET, (uint32_t)args->arg1,
                                    (uint32_t)args->arg2, (uint32_t)args->arg3,
                                    (uint32_t)args->arg4, (uint32_t)args->arg5,
                                    (uint32_t)args->arg6, (uint32_t)args->arg7,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void val_ffa_notification_info_get_32(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_NOTIFICATION_INFO_GET_32, (uint32_t)args->arg1,
                                    (uint32_t)args->arg2, (uint32_t)args->arg3,
                                    (uint32_t)args->arg4, (uint32_t)args->arg5,
                                    (uint32_t)args->arg6, (uint32_t)args->arg7,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void val_ffa_notification_info_get_64(ffa_args_t *args)
{
    *args = ffa_smccc(FFA_NOTIFICATION_INFO_GET_64, args->arg1,
                                    args->arg2, args->arg3,
                                    args->arg4, args->arg5,
                                    args->arg6, args->arg7,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void ffa_mem_perm_set(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_MEM_PERM_SET_64, args->arg1, args->arg2, args->arg3,
                                args->arg4, args->arg5, args->arg6, args->arg7,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_MEM_PERM_SET_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

void val_ffa_mem_perm_set_32(ffa_args_t *args)
{
    ffa_mem_perm_set(args, false);
}

void val_ffa_mem_perm_set_64(ffa_args_t *args)
{
    ffa_mem_perm_set(args, true);
}

static void ffa_mem_perm_get(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_MEM_PERM_GET_64, args->arg1, args->arg2, args->arg3,
                                args->arg4, args->arg5, args->arg6, args->arg7,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        *args = ffa_smccc(FFA_MEM_PERM_GET_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

void val_ffa_mem_perm_get_32(ffa_args_t *args)
{
    ffa_mem_perm_get(args, false);
}

void val_ffa_mem_perm_get_64(ffa_args_t *args)
{
    ffa_mem_perm_get(args, true);
}


/**
 * @brief - Maps the RX/TX buffer pair in the callee's translation regime
 *          on behalf of an endpoint.
 * @param tx_buf - TX buffer address.
 * @param rx_buf - RX buffer address.
 * @param page_count - 4K page count.
 * @return - Returns status code.
**/
uint32_t val_rxtx_map_64(uint64_t tx_buf, uint64_t rx_buf, uint32_t page_count)
{
    ffa_args_t payload;


    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint64_t)val_mem_virt_to_phys((void *)tx_buf);
    payload.arg2 = (uint64_t)val_mem_virt_to_phys((void *)rx_buf);
    payload.arg3 = page_count;

    val_ffa_rxtx_map_64(&payload);

    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "RXTX_MAP failed err 0x%x", payload.arg2);
        return VAL_ERROR;
    }

    return VAL_SUCCESS;
}

/**
 * @brief - Maps the RX/TX buffer pair in the callee's translation regime
 *          on behalf of an endpoint.
 * @param tx_buf - TX buffer address.
 * @param rx_buf - RX buffer address.
 * @param page_count - 4K page count.
 * @return - Returns status code.
**/
uint32_t val_rxtx_map_32(uint64_t tx_buf, uint64_t rx_buf, uint32_t page_count)
{
    ffa_args_t payload;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint64_t)val_mem_virt_to_phys((void *)tx_buf);
    payload.arg2 = (uint64_t)val_mem_virt_to_phys((void *)rx_buf);
    payload.arg3 = page_count;

    val_ffa_rxtx_map_32(&payload);

    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "RXTX_MAP failed err 0x%x", payload.arg2);
        return VAL_ERROR;
    }
    return VAL_SUCCESS;
}

/**
 * @brief - Unmaps an endpoint's RX/TX buffer pair from the callee's
            translation regime.
 * @param id - Endpoint id. Currently unused.
 * @return - Returns status code.
**/
uint32_t val_rxtx_unmap(ffa_endpoint_id_t id)
{
    ffa_args_t payload;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_rxtx_unmap(&payload);

    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "RXTX_UNMAP failed err 0x%x", payload.arg2);
        return VAL_ERROR;
    }
    (void)id;
    return VAL_SUCCESS;
}

/**
 * @brief - Relinquish ownership of a RX buffer after reading a message from it.
 * @param - void
 * @return - Returns status code.
**/
uint32_t val_rx_release(void)
{
    ffa_args_t payload;

    /* Release the RX buffer */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    val_ffa_rx_release(&payload);

    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "RX_RELEASE failed err 0x%x", payload.arg2);
        return VAL_ERROR;
    }
    return VAL_SUCCESS;
}

/**
 * @brief - Check the reserved parameters are zero, if not return error.
 * @param args - Parameter structure.
 * @param param_count - Reserve parameter count.
 * @return - Returns status code.
**/
uint32_t val_reserve_param_check(ffa_args_t args, uint32_t param_count)
{
    uint32_t total_count = 8;
    uint32_t count = total_count - param_count;
    uint64_t *payload = (uint64_t *)&args;

    for (; count < 8; count++)
    {
        if (payload[count])
            return VAL_ERROR;
    }

    return VAL_SUCCESS;
}

static void ffa_console_log(ffa_args_t *args, bool arch64)
{
    if (arch64)
    {
        *args = ffa_smccc(FFA_CONSOLE_LOG_64, args->arg1, args->arg2,
                        args->arg3, args->arg4, args->arg5, args->arg6,
                        args->arg7, args->ext_args.arg8, args->ext_args.arg9, args->ext_args.arg10,
                        args->ext_args.arg11, args->ext_args.arg12, args->ext_args.arg13,
                        args->ext_args.arg14, args->ext_args.arg15, args->ext_args.arg16,
                        args->ext_args.arg17);
    }
    else
    {
        *args = ffa_smccc(FFA_CONSOLE_LOG_32, (uint32_t)args->arg1,
                          (uint32_t)args->arg2, (uint32_t)args->arg3,
                          (uint32_t)args->arg4, (uint32_t)args->arg5,
                          (uint32_t)args->arg6, (uint32_t)args->arg7,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}

/**
 * @brief - Starts a transaction to grant access to a memory region
 *          to one or more Borrowers.
 * @param args - Input arguments to FFA_MEM_SHARE_32 abi.
 * @return - Returns success/error status code in response to FFA_MEM_SHARE abi.
**/
void val_ffa_console_log_32(ffa_args_t *args)
{
    ffa_console_log(args, false);
}

/**
 * @brief - Starts a transaction to grant access to a memory region
 *          to one or more Borrowers.
 * @param args - Input arguments to FFA_MEM_SHARE_64 abi.
 * @return - Returns success/error status code in response to FFA_MEM_SHARE abi
**/
void val_ffa_console_log_64(ffa_args_t *args)
{
    ffa_console_log(args, true);
}