/*
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define REFCLK_TIMEOUT 100000U
#define IRQ_TRIGGERED  0xABCDABCD

static uint32_t *pages;
static int refclk_irq_handler(void)
{
    *(volatile uint32_t *)pages = (uint32_t)IRQ_TRIGGERED;
    LOG(DBG, "REF-CLK IRQ Handler Processed");
    return 0;
}


uint32_t other_secure_int6_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t receiver = val_get_endpoint_id(server_logical_id);
    ffa_endpoint_id_t receiver_1 = val_get_endpoint_id(SP1);
    mb_buf_t mb;
    uint64_t size = 0x1000;
    ffa_memory_region_flags_t flags = 0;
    ffa_memory_handle_t handle;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    if (val_is_ffa_feature_supported(FFA_MEM_SHARE_32))
    {
        LOG(TEST, "FFA_MEM_SHARE_32 not supported, skipping the test");
        return VAL_SKIP_CHECK;
    }

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }
    val_memset(mb.send, 0, size);

    pages = (uint32_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        status = VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }
    val_memset(pages, 0, size);

    if (val_irq_register_handler(PALTFORM_AP_REFCLK_CNTPSIRQ1, refclk_irq_handler))
    {
        LOG(ERROR, "AP_REFCLK interrupt register failed");
        status = VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = receiver_1;
    mem_region_init.tag = 0;
    mem_region_init.flags = flags;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
#if (PLATFORM_OUTER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#elif (PLATFORM_INNER_SHAREABLE_SUPPORT_ONLY == 1)
    mem_region_init.shareability = FFA_MEMORY_INNER_SHAREABLE;
#elif (PLATFORM_INNER_OUTER_SHAREABLE_SUPPORT == 1)
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
#endif
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;
    val_ffa_mem_share_32(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_share request failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(5);
        goto free_interrupt;
    }

    handle = ffa_mem_success_handle(payload);

    /* Enable System Timer Interrupt. */
    val_sys_phy_timer_en(REFCLK_TIMEOUT);
    LOG(DBG, "System timer enabled");

    /* Pass memory handle to the server using direct message */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    payload.arg3 =  handle;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct request failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(6);
        goto free_interrupt;
    }

    /* Disable System Timer Interrupt. */
    val_sys_phy_timer_dis(true);
    LOG(DBG, "System timer disabled");

    /* Should receive interrupt on preempt -> running. */
    if (*(volatile uint32_t *)pages != IRQ_TRIGGERED)
    {
        LOG(ERROR, "WD interrupt should be triggered");
        status =  VAL_ERROR_POINT(7);
        goto free_interrupt;
    }
    LOG(DBG, "IRQ status Pages %x", pages);

    /* Schedule the preempted SP using FFA_RUN */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)receiver << 16;
    LOG(DBG, "Call FFA Run");
    val_ffa_run(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_64)
    {
        LOG(ERROR, "DIRECT_RESP_64 not received fid %x", payload.fid);
        status = VAL_ERROR_POINT(8);
        goto free_interrupt;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)handle;
    payload.arg2 = (uint32_t)(handle >> 32);
    payload.arg3 = 0;
    val_ffa_mem_reclaim(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem Reclaim failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(9);
    }
    LOG(DBG, "FFA Mem Reclaim Complete");

free_interrupt:
    if (val_irq_unregister_handler(PALTFORM_AP_REFCLK_CNTPSIRQ1))
    {
        LOG(ERROR, "IRQ handler unregister failed");
        status = VAL_ERROR_POINT(10);
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = VAL_ERROR_POINT(11);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(12);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(13);
    }

    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);
    return status ? status : (uint32_t)payload.arg3;
}

