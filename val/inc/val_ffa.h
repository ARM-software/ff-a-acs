/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_FFA_H_
#define _VAL_FFA_H_

#define FFA_VERSION_MAJOR 0x1
#if (PLATFORM_FFA_V == FFA_V_1_0)
#define FFA_VERSION_MINOR 0x0
#elif (PLATFORM_FFA_V == FFA_V_1_1)
#define FFA_VERSION_MINOR 0x1
#else
#define FFA_VERSION_MINOR 0x2
#endif

#define VAL_GET_MAJOR(x) ((x >> 16) & 0x7fff)
#define VAL_GET_MINOR(x) (x & 0xffff)

/* FF-A Error Codes. */
#define FFA_ERROR_NOT_SUPPORTED      0xffffffff //-1
#define FFA_ERROR_INVALID_PARAMETERS 0xfffffffe //-2
#define FFA_ERROR_NO_MEMORY          0xfffffffd //-3
#define FFA_ERROR_BUSY               0xfffffffc //-4
#define FFA_ERROR_INTERRUPTED        0xfffffffb //-5
#define FFA_ERROR_DENIED             0xfffffffa //-6
#define FFA_ERROR_RETRY              0xfffffff9 //-7
#define FFA_ERROR_ABORTED            0xfffffff8 //-8
#define FFA_ERROR_NODATA             0xfffffff7 //-9

/* FFA 32 bit- function ids. */
#define FFA_ERROR_32                 0x84000060
#define FFA_SUCCESS_32               0x84000061
#define FFA_INTERRUPT_32             0x84000062
#define FFA_VERSION_32               0x84000063
#define FFA_FEATURES_32              0x84000064
#define FFA_RX_RELEASE_32            0x84000065
#define FFA_RXTX_MAP_32              0x84000066
#define FFA_RXTX_UNMAP_32            0x84000067
#define FFA_PARTITION_INFO_GET_32    0x84000068
#define FFA_ID_GET_32                0x84000069
#define FFA_MSG_POLL_32              0x8400006A
#define FFA_MSG_WAIT_32              0x8400006B
#define FFA_YIELD_32                 0x8400006C
#define FFA_RUN_32                   0x8400006D
#define FFA_MSG_SEND_32              0x8400006E
#define FFA_MSG_SEND_DIRECT_REQ_32   0x8400006F
#define FFA_MSG_SEND_DIRECT_RESP_32  0x84000070
#define FFA_MEM_DONATE_32            0x84000071
#define FFA_MEM_LEND_32              0x84000072
#define FFA_MEM_SHARE_32             0x84000073
#define FFA_MEM_RETRIEVE_REQ_32      0x84000074
#define FFA_MEM_RETRIEVE_RESP_32     0x84000075
#define FFA_MEM_RELINQUISH_32        0x84000076
#define FFA_MEM_RECLAIM_32           0x84000077
#define FFA_NORMAL_WORLD_RESUME_32   0x8400007C
#define FFA_SECONDARY_EP_REGISTER_32 0x84000087
#define FFA_SPM_ID_GET_32            0x84000085
#define FFA_MSG_SEND2_32             0x84000086
#define FFA_MEM_PERM_GET_32          0x84000088
#define FFA_MEM_PERM_SET_32          0x84000089
#define FFA_CONSOLE_LOG_32           0x8400008A

#define FFA_NOTIFICATION_BITMAP_CREATE   0x8400007D
#define FFA_NOTIFICATION_BITMAP_DESTROY  0x8400007E
#define FFA_NOTIFICATION_BIND            0x8400007F
#define FFA_NOTIFICATION_UNBIND          0x84000080
#define FFA_NOTIFICATION_SET             0x84000081
#define FFA_NOTIFICATION_GET             0x84000082
#define FFA_NOTIFICATION_INFO_GET_32     0x84000083

/* FFA 64 bit- function identifiers. */
#define FFA_SUCCESS_64               0xC4000061
#define FFA_RXTX_MAP_64              0xC4000066
#define FFA_MSG_SEND_DIRECT_REQ_64   0xC400006F
#define FFA_MSG_SEND_DIRECT_RESP_64  0xC4000070
#define FFA_MEM_DONATE_64            0xC4000071
#define FFA_MEM_LEND_64              0xC4000072
#define FFA_MEM_SHARE_64             0xC4000073
#define FFA_MEM_RETRIEVE_REQ_64      0xC4000074
#define FFA_SECONDARY_EP_REGISTER_64 0xC4000087
#define FFA_NOTIFICATION_INFO_GET_64 0xC4000083
#define FFA_MEM_PERM_GET_64          0xC4000088
#define FFA_MEM_PERM_SET_64          0xC4000089
#define FFA_CONSOLE_LOG_64           0xC400008A
#define FFA_MSG_SEND_DIRECT_REQ2_64  0xC400008D
#define FFA_MSG_SEND_DIRECT_RESP2_64 0xC400008E
#define FFA_PARTITION_INFO_GET_REGS_64    0xC400008B

#define SENDER_ID(x)    (x >> 16) & 0xffff
#define RECEIVER_ID(x)  (x & 0xffff)

#define FFA_RXTX_MAP_4K_SIZE         0x0
#define FFA_RXTX_MAP_64K_SIZE        0x1
#define FFA_RXTX_MAP_16K_SIZE        0x2
#define FFA_DYNAMIC_BUFFER_SUPPORT   0X1

#define VAL_EXTRACT_BITS(data, start, end) ((data >> start) & ((1ul << (end - start + 1)) - 1))

/* 16-bit ID of FF-A component */
typedef uint16_t ffa_endpoint_id_t;

/* Parameters for FF-A ABI calls */
typedef struct {
    uint64_t fid;
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
    uint64_t arg4;
    uint64_t arg5;
    uint64_t arg6;
    uint64_t arg7;
    struct {
        uint64_t arg8;
        uint64_t arg9;
        uint64_t arg10;
        uint64_t arg11;
        uint64_t arg12;
        uint64_t arg13;
        uint64_t arg14;
        uint64_t arg15;
        uint64_t arg16;
        uint64_t arg17;
    } ext_args;
} ffa_args_t;

typedef struct mailbox_buffers {
    void *recv;
    void *send;
} mb_buf_t;

typedef struct ffa_partition_info {
    /** The ID of the VM the information is about */
    ffa_endpoint_id_t id;
    /** The number of execution contexts implemented by the partition */
    uint16_t exec_context;
    /** The Partition's properties, e.g. supported messaging methods */
    uint32_t properties;
#if (PLATFORM_FFA_V >= FFA_V_1_1)
	uint32_t uuid[4];
#endif
} ffa_partition_info_t;

void val_call_conduit(ffa_args_t *args);
void val_call_conduit_ext(ffa_args_t *args);
void val_ffa_error(ffa_args_t *args);
void val_ffa_success_32(ffa_args_t *args);
void val_ffa_success_64(ffa_args_t *args);
void val_ffa_version(ffa_args_t *args);
void val_ffa_msg_send_direct_req_32(ffa_args_t *args);
void val_ffa_msg_send_direct_req_64(ffa_args_t *args);
void val_ffa_msg_send_direct_req2_64(ffa_args_t *args);
void val_ffa_msg_send_direct_resp_32(ffa_args_t *args);
void val_ffa_msg_send_direct_resp_64(ffa_args_t *args);
void val_ffa_msg_send_direct_resp2_64(ffa_args_t *args);
ffa_endpoint_id_t val_get_curr_endpoint_id(void);
void val_ffa_id_get(ffa_args_t *args);
void val_ffa_spm_id_get(ffa_args_t *args);
void val_ffa_rx_release(ffa_args_t *args);
void val_ffa_rxtx_unmap(ffa_args_t *args);
void val_ffa_rxtx_map_32(ffa_args_t *args);
void val_ffa_rxtx_map_64(ffa_args_t *args);
void val_ffa_msg_send(ffa_args_t *args);
void val_ffa_msg_send2(ffa_args_t *args);
void val_ffa_partition_info_get(ffa_args_t *args);
void val_ffa_partition_info_get_regs(ffa_args_t *args);
void val_ffa_features(ffa_args_t *args);
void val_ffa_memory_reclaim(ffa_args_t *args);
void val_ffa_msg_wait(ffa_args_t *args);
void val_ffa_yield(ffa_args_t *args);
void val_ffa_run(ffa_args_t *args);
void val_ffa_msg_poll(ffa_args_t *args);
void val_ffa_mem_donate_32(ffa_args_t *args);
void val_ffa_mem_donate_64(ffa_args_t *args);
void val_ffa_mem_lend_32(ffa_args_t *args);
void val_ffa_mem_lend_64(ffa_args_t *args);
void val_ffa_mem_share_32(ffa_args_t *args);
void val_ffa_mem_share_64(ffa_args_t *args);
void val_ffa_mem_retrieve_32(ffa_args_t *args);
void val_ffa_mem_retrieve_64(ffa_args_t *args);
void val_ffa_mem_relinquish(ffa_args_t *args);
void val_ffa_mem_reclaim(ffa_args_t *args);
uint32_t val_rxtx_map_64(uint64_t tx_buf, uint64_t rx_buf, uint32_t page_count);
uint32_t val_rxtx_map_32(uint64_t tx_buf, uint64_t rx_buf, uint32_t page_count);
uint32_t val_rxtx_unmap(ffa_endpoint_id_t id);
uint32_t val_rx_release(void);
uint32_t val_reserve_param_check(ffa_args_t args, uint32_t param_count);
void val_ffa_secondary_ep_register_64(void);
void val_ffa_notification_bitmap_create(ffa_args_t *args);
void val_ffa_notification_bitmap_destroy(ffa_args_t *args);
void val_ffa_notification_bind(ffa_args_t *args);
void val_ffa_notification_unbind(ffa_args_t *args);
void val_ffa_notification_set(ffa_args_t *args);
void val_ffa_notification_get(ffa_args_t *args);
void val_ffa_notification_info_get_32(ffa_args_t *args);
void val_ffa_notification_info_get_64(ffa_args_t *args);
void val_ffa_mem_perm_get_64(ffa_args_t *args);
void val_ffa_mem_perm_get_32(ffa_args_t *args);
void val_ffa_mem_perm_set_64(ffa_args_t *args);
void val_ffa_mem_perm_set_32(ffa_args_t *args);
void val_ffa_console_log_32(ffa_args_t *args);
void val_ffa_console_log_64(ffa_args_t *args);
#endif /* _VAL_FFA_H_ */
