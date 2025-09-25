/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static int g_handler_lower;
static int g_handler_upper;
static uint8_t *ptr;

/**
 * Incase PM injects data abort to lower EL,
 * Handles the data abort at EL1.
 */
static bool exception_handler_data_abort_lower(void)
{
    uint64_t esr_el1 = val_esr_el1_read();
    uint64_t far_el1 = val_far_el1_read();
    uint64_t next_pc = val_elr_el1_read() + 4;
    uint64_t ec = esr_el1 >> 26;

    if (ec != EC_DATA_ABORT_SAME_EL  || far_el1 != (uint64_t)(ptr - 1))
    {
        LOG(ERROR, "Unexpected exception detected ec=%x, far=%x\n", ec, far_el1);
    }
    else
    {
        LOG(INFO, "Expected exception detected\n");
        g_handler_lower = 1;
    }

    /* Skip instruction that triggered the exception. */
    val_elr_el1_write(next_pc);

    /* Reset reboot to catch unwanted hang */
    val_reset_reboot_flag();

    /* Indicate that elr_el1 should not be restored. */
    return true;
}

static bool exception_handler_data_abort_upper(void)
{
    uint64_t esr_el1 = val_esr_el1_read();
    uint64_t far_el1 = val_far_el1_read();
    uint64_t next_pc = val_elr_el1_read() + 4;
    uint64_t ec = esr_el1 >> 26;

    if (ec != EC_DATA_ABORT_SAME_EL  || far_el1 != (uint64_t)(ptr + 0x1000))
    {
        LOG(ERROR, "Unexpected exception detected ec=%x, far=%x\n", ec, far_el1);
    }
    else
    {
        LOG(INFO, "Expected exception detected\n");
        g_handler_upper = 1;
    }

    /* Skip instruction that triggered the exception. */
    val_elr_el1_write(next_pc);

    /* Reset reboot to catch unwanted hang */
    val_reset_reboot_flag();

    /* Indicate that elr_el1 should not be restored. */
    return true;
}

static uint32_t donate_lower_upper_boundary_32_server(ffa_args_t args)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS, violation_status = VAL_ERROR;
    ffa_endpoint_id_t sender = args.arg1 & 0xffff;
    ffa_endpoint_id_t receiver = (args.arg1 >> 16) & 0xffff;
    mb_buf_t mb;
    uint64_t size = 0x1000;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region *memory_region;
    struct ffa_composite_memory_region *composite;
    ffa_memory_handle_t handle;
    uint32_t msg_size;
    memory_region_descriptor_t mem_desc;
    struct ffa_memory_region_constituent constituents[2];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    mb.send = val_aligned_alloc(PAGE_SIZE_4K, size);
    mb.recv = val_aligned_alloc(PAGE_SIZE_4K, size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer\n");
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed\n");
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }
    val_memset(mb.send, 0, size);

    /* Wait for the message. */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload = val_resp_client_fn_direct((uint32_t)args.arg3, 0, 0, 0, 0, 0);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_64)
    {
        LOG(ERROR, "Direct request failed, fid=0x%x, err 0x%x\n",
                  payload.fid, payload.arg2);
        status =  VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

    handle = payload.arg3;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = receiver;
    mem_region_init.receiver = sender;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NX;
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

    msg_size = val_ffa_memory_retrieve_request_init(&mem_region_init, handle);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = msg_size;
    payload.arg2 = msg_size;
    val_ffa_mem_retrieve_32(&payload);
    if (payload.fid != FFA_MEM_RETRIEVE_RESP_32)
    {
        LOG(ERROR, "Mem retrieve request failed err %x\n", payload.arg2);
        status =  VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }

    memory_region = (struct ffa_memory_region *)mb.recv;
    composite = ffa_memory_region_get_composite(memory_region, 0);
    ptr = (uint8_t *)composite->constituents[0].address;

    /* Map the lower page and retrieve region into endpoint translation */
    mem_desc.virtual_address = (uint64_t)composite->constituents[0].address - size;
    mem_desc.physical_address = (uint64_t)composite->constituents[0].address - size;
    mem_desc.length = (size * 2);
    if (VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(receiver)))
        mem_desc.attributes = ATTR_RW_DATA;
    else
        mem_desc.attributes = ATTR_RW_DATA | ATTR_NS;

    if (val_mem_map_pgt(&mem_desc))
    {
        LOG(ERROR, "Va to pa mapping failed\n");
        status =  VAL_ERROR_POINT(5);
        goto mem_donate;
    }

    /* Register fault handler incase PM injects/forwards the abort to lower EL */
    val_exception_setup(NULL, exception_handler_data_abort_lower);

    /* Set reboot flag in case system resets on detection of access violation */
    val_set_reboot_flag();

    /*
     * Lower boundary check: Check that we can't access out of bounds before the region sent to
     * us. This should trigger the exception handler.
     */
    ptr[-1]++;

    /* Unregister fault handler */
    val_exception_setup(NULL, NULL);

    /* Reset reboot flag to catch unwanted hang */
    val_reset_reboot_flag();

    ptr = (uint8_t *)composite->constituents[1].address;
    /* Map the upper page and retrieve region into endpoint translation */
    mem_desc.virtual_address = (uint64_t)composite->constituents[1].address;
    mem_desc.physical_address = (uint64_t)composite->constituents[1].address;
    mem_desc.length = (size * 2);
    if (VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(receiver)))
        mem_desc.attributes = ATTR_RW_DATA;
    else
        mem_desc.attributes = ATTR_RW_DATA | ATTR_NS;

    if (val_mem_map_pgt(&mem_desc))
    {
        LOG(ERROR, "Va to pa mapping failed\n");
        status =  VAL_ERROR_POINT(6);
        goto mem_donate;
    }

    /* Register fault handler incase PM injects/forwards the abort to lower EL */
    val_exception_setup(NULL, exception_handler_data_abort_upper);

    /* Set reboot flag in case system resets on detection of access violation */
    val_set_reboot_flag();

    /*
     * Upper boundary check: Check that we can't access out of bounds before the region sent to
     * us. This should trigger the exception handler.
     */
    ptr[size]++;

    /* Unregister fault handler */
    val_exception_setup(NULL, NULL);

    /* Reset reboot flag to catch unwanted hang */
    val_reset_reboot_flag();
    /* Check for lower and upper data aborts */
    if (g_handler_lower && g_handler_upper)
        violation_status = VAL_SUCCESS;
    else
        violation_status = VAL_ERROR_POINT(7);

    g_handler_lower = 0;
    g_handler_upper = 0;

mem_donate:
    constituents[0].address = composite->constituents[0].address;
    constituents[0].page_count = 1;
    constituents[1].address = composite->constituents[1].address;
    constituents[1].page_count = 1;

    /* Give up ownership back to the sender */
    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = receiver;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_NOT_SPECIFIED;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NOT_SPECIFIED_MEM;
    mem_region_init.cacheability = 0;
    mem_region_init.shareability = 0;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;
    val_ffa_mem_donate_32(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_donate request failed err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(8);
        goto rx_release;
    }
    LOG(DBG, "Mem Donate Complete\n");

    handle = ffa_mem_success_handle(payload);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    payload.arg3 =  handle;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
    {
        LOG(ERROR, "Direct request failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(9);
    }

rx_release:
    if (val_rx_release())
    {
        LOG(ERROR, "val_rx_release failed\n");
        status = status ? status : VAL_ERROR_POINT(10);
    }
rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed\n");
        status = status ? status : VAL_ERROR_POINT(11);
    }
free_memory:
    if (val_free(mb.recv) || val_free(mb.send))
    {
        LOG(ERROR, "free_rxtx_buffers failed\n");
        status = status ? status : VAL_ERROR_POINT(12);
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | receiver;
    val_ffa_msg_send_direct_resp_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct response failed err %x\n", payload.arg2);
        status = status ? status : VAL_ERROR_POINT(13);
    }

    return violation_status ? violation_status : status;
}

uint32_t donate_lower_upper_boundary_32_spsp_server(ffa_args_t args)
{
    return donate_lower_upper_boundary_32_server(args);
}

uint32_t donate_lower_upper_boundary_32_vmvm_server(ffa_args_t args)
{
    return donate_lower_upper_boundary_32_server(args);
}
