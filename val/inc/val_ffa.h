/*
 * Copyright (c) 2021-2024, 2026, Arm Limited or its affiliates. All rights reserved.
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


/* ffa_fid_list */
#define FFA_FID_LIST(X) \
    /* FF-A 32-bit function IDs */ \
    X(FFA_ERROR_32,                 0x84000060) \
    X(FFA_SUCCESS_32,               0x84000061) \
    X(FFA_INTERRUPT_32,             0x84000062) \
    X(FFA_VERSION_32,               0x84000063) \
    X(FFA_FEATURES_32,              0x84000064) \
    X(FFA_RX_RELEASE_32,            0x84000065) \
    X(FFA_RXTX_MAP_32,              0x84000066) \
    X(FFA_RXTX_UNMAP_32,            0x84000067) \
    X(FFA_PARTITION_INFO_GET_32,    0x84000068) \
    X(FFA_ID_GET_32,                0x84000069) \
    X(FFA_MSG_POLL_32,              0x8400006A) \
    X(FFA_MSG_WAIT_32,              0x8400006B) \
    X(FFA_YIELD_32,                 0x8400006C) \
    X(FFA_RUN_32,                   0x8400006D) \
    X(FFA_MSG_SEND_32,              0x8400006E) \
    X(FFA_MSG_SEND_DIRECT_REQ_32,   0x8400006F) \
    X(FFA_MSG_SEND_DIRECT_RESP_32,  0x84000070) \
    X(FFA_MEM_DONATE_32,            0x84000071) \
    X(FFA_MEM_LEND_32,              0x84000072) \
    X(FFA_MEM_SHARE_32,             0x84000073) \
    X(FFA_MEM_RETRIEVE_REQ_32,      0x84000074) \
    X(FFA_MEM_RETRIEVE_RESP_32,     0x84000075) \
    X(FFA_MEM_RELINQUISH_32,        0x84000076) \
    X(FFA_MEM_RECLAIM_32,           0x84000077) \
    X(FFA_NORMAL_WORLD_RESUME_32,   0x8400007C) \
    \
    X(FFA_NOTIFICATION_BITMAP_CREATE,  0x8400007D) \
    X(FFA_NOTIFICATION_BITMAP_DESTROY, 0x8400007E) \
    X(FFA_NOTIFICATION_BIND,           0x8400007F) \
    X(FFA_NOTIFICATION_UNBIND,         0x84000080) \
    X(FFA_NOTIFICATION_SET,            0x84000081) \
    X(FFA_NOTIFICATION_GET,            0x84000082) \
    X(FFA_NOTIFICATION_INFO_GET_32,    0x84000083) \
    \
    X(FFA_RX_ACQUIRE_32,            0x84000084) \
    X(FFA_SPM_ID_GET_32,            0x84000085) \
    X(FFA_MSG_SEND2_32,             0x84000086) \
    X(FFA_SECONDARY_EP_REGISTER_32, 0x84000087) \
    X(FFA_MEM_PERM_GET_32,          0x84000088) \
    X(FFA_MEM_PERM_SET_32,          0x84000089) \
    X(FFA_CONSOLE_LOG_32,           0x8400008A) \
    \
    /* FF-A 64-bit function IDs */ \
    X(FFA_SUCCESS_64,               0xC4000061) \
    X(FFA_RXTX_MAP_64,              0xC4000066) \
    X(FFA_MSG_SEND_DIRECT_REQ_64,   0xC400006F) \
    X(FFA_MSG_SEND_DIRECT_RESP_64,  0xC4000070) \
    X(FFA_MEM_DONATE_64,            0xC4000071) \
    X(FFA_MEM_LEND_64,              0xC4000072) \
    X(FFA_MEM_SHARE_64,             0xC4000073) \
    X(FFA_MEM_RETRIEVE_REQ_64,      0xC4000074) \
    X(FFA_SECONDARY_EP_REGISTER_64, 0xC4000087) \
    X(FFA_NOTIFICATION_INFO_GET_64, 0xC4000083) \
    X(FFA_MEM_PERM_GET_64,          0xC4000088) \
    X(FFA_MEM_PERM_SET_64,          0xC4000089) \
    X(FFA_CONSOLE_LOG_64,           0xC400008A) \
    X(FFA_MSG_SEND_DIRECT_REQ2_64,  0xC400008D) \
    X(FFA_MSG_SEND_DIRECT_RESP2_64, 0xC400008E) \
    X(FFA_PARTITION_INFO_GET_REGS_64, 0xC400008B)

/* Masks to separate ABI information and function number from FF-A FIDs */
#define FFA_FID_UPPER_MASK        0xFFFF0000
#define FFA_FID_LOWER_MASK        0x0000FFFF


/* ABI prefixes encoded in the upper bits of FF-A function IDs */
#define FFA_FID_ABI32_PREFIX      0x84000000
#define FFA_FID_ABI64_PREFIX      0xC4000000


/* Special marker used by VAL to skip ABI checking (not a real FF-A FID) */
#define FFA_FID_SKIP_CHECK        0xABCDEFFF


/* ANY-ABI FF-A FIDs:
 * Used for tests that must run on both 32-bit and 64-bit ABIs.
 * The ABI bits are masked so only the function number is matched.
 */
#define FFA_MEM_SHARE_ANY                 (FFA_MEM_SHARE_32 & FFA_FID_LOWER_MASK)
#define FFA_MEM_LEND_ANY                  (FFA_MEM_LEND_32  & FFA_FID_LOWER_MASK)
#define FFA_MEM_DONATE_ANY                (FFA_MEM_DONATE_32 & FFA_FID_LOWER_MASK)
#define FFA_MEM_RETRIEVE_REQ_ANY          (FFA_MEM_RETRIEVE_REQ_32 & FFA_FID_LOWER_MASK)
#define FFA_MSG_SEND_DIRECT_REQ_ANY       (FFA_MSG_SEND_DIRECT_REQ_32 & FFA_FID_LOWER_MASK)
#define FFA_NOTIFICATION_INFO_GET_ANY     (FFA_NOTIFICATION_INFO_GET_32 & FFA_FID_LOWER_MASK)
#define FFA_CONSOLE_LOG_ANY               (FFA_CONSOLE_LOG_32 & FFA_FID_LOWER_MASK)


/* FF-A Function Enum*/
typedef enum {
#define X(name, val) name = val,
    FFA_FID_LIST(X)
#undef X
} ffa_func_id_t;

/* FF-A Function Array */
static const ffa_func_id_t ffa_supported_fids[] = {
#define X(name, val) name,
    FFA_FID_LIST(X)
#undef X
};

#define FFA_SUPPORTED_FID_COUNT \
    (sizeof(ffa_supported_fids) / sizeof(ffa_supported_fids[0]))


#define SENDER_ID(x)    (x >> 16) & 0xffff
#define RECEIVER_ID(x)  (x & 0xffff)

#define FFA_RXTX_MAP_4K_SIZE         0x0
#define FFA_RXTX_MAP_64K_SIZE        0x1
#define FFA_RXTX_MAP_16K_SIZE        0x2
#define FFA_DYNAMIC_BUFFER_SUPPORT   0X1

#define VAL_EXTRACT_BITS(data, start, end) ((data >> start) & ((1ul << (end - start + 1)) - 1))

#define FFA_PARTITION_INFO_V1_0_SIZE   8U
#define FFA_PARTITION_INFO_V1_1_SIZE   24U
#define FFA_PARTITION_INFO_V1_2_SIZE   24U
#define FFA_PARTITION_INFO_V1_3_SIZE   48U

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


/*
 * FF-A Partition Information Descriptor
 *
 * Based on:
 *  - FF-A v1.0  : Basic partition identity
 *  - FF-A v1.1+ : Adds protocol UUID
 *  - FF-A v1.3  : Adds extended metadata (image UUID, versioning)
 *
 * Descriptor size by version:
 *  - v1.0 : 8 bytes
 *  - v1.1 / v1.2 : 24 bytes
 *  - v1.3 : 48 bytes
 *
 * NOTE:
 *  - Newer implementations (e.g., SPMC v1.3) may return larger descriptors
 *    even if the endpoint supports an older version.
 *  - Consumers MUST use the descriptor size returned by the ABI rather than
 *    assuming a fixed version.
 */

typedef struct ffa_partition_info {

        /* v1.0 */
        ffa_endpoint_id_t id;   /* VM ID */
        uint16_t exec_context;  /* vCPU count */
        uint32_t properties;    /* flags */

        /* v1.1 v1.2 */
        uint32_t uuid[4];     /* protocol UUID */

        /* v1.3 */
        uint32_t image_uuid[4];        /* image UUID */
        uint32_t partition_ffa_version;    /* FF-A version */
        uint32_t reserved_0;               /* reserved */
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
