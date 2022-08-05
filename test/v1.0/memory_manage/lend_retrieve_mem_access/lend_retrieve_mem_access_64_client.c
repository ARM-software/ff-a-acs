/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static int g_handler = 0;
static uint8_t *ptr;

/**
 * Incase PM injects data abort to lower EL,
 * Handles the data abort at EL1.
 */

static bool exception_handler_data_abort(void)
{
    uint64_t esr_el1 = val_esr_el1_read();
    uint64_t far_el1 = val_far_el1_read();
    uint64_t next_pc = val_elr_el1_read() + 4;
    uint64_t ec = esr_el1 >> 26;

    if (ec != EC_DATA_ABORT_SAME_EL  || far_el1 != (uint64_t)ptr)
    {
        LOG(ERROR, "\tUnexpected exception detected ec=%x, far=%x\n", ec, far_el1);
    }
    else
    {
        LOG(INFO, "\tExpected exception detected\n", 0, 0);
        g_handler = 1;
    }

    /* Skip instruction that triggered the exception. */
    val_elr_el1_write(next_pc);

    /* Reset reboot to catch unwanted hang */
    val_reset_reboot_flag();

    /* Indicate that elr_el1 should not be restored. */
    return true;
}

static uint32_t lend_retrieve_mem_access(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);
    mb_buf_t mb;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    ffa_memory_region_flags_t flags = 0;
    ffa_memory_handle_t handle;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    if (val_is_ffa_feature_supported(FFA_MEM_LEND_64))
    {
        LOG(TEST, "\tFFA_MEM_LEND_64 not supported, skipping the check\n", 0, 0);
        return VAL_SKIP_CHECK;
    }

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "\tRxTx Map failed\n", 0, 0);
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "\tMemory allocation failed\n", 0, 0);
        status = VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    ptr = pages;

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = flags;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NOT_SPECIFIED_MEM;
    mem_region_init.cacheability = 0;
    mem_region_init.shareability = 0;
    mem_region_init.multi_share = false;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    /* MEM_LEND executes successfully, the Relayer must ensure that the state of memory region
     * Owner-NA for the Lender.
     */
    val_ffa_mem_lend_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tMem_lend request failed err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }

    handle = ffa_mem_success_handle(payload);

    /* Pass memory handle to the server using direct message */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    payload.arg3 =  handle;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tDirect request failed err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

    /* Register fault handler incase PM injects/forwards the abort to lower EL */
    val_exception_setup(NULL, exception_handler_data_abort);

    /* Set reboot flag in case system resets on detection of access violation */
    val_set_reboot_flag();

    /* Check memory write access violation */
    ++ptr[0];

    /* Unregister fault handler */
    val_exception_setup(NULL, NULL);

    /* Reset reboot flag to catch unwanted hang */
    val_reset_reboot_flag();

    if (g_handler)
        status = VAL_SUCCESS;
    else
        status = VAL_ERROR_POINT(5);

    g_handler = 0;

    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)handle;
    payload.arg2 = (uint32_t)(handle >> 32);
    payload.arg3 = 0;
    val_ffa_mem_reclaim(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "\tMem Reclaim failed err %x\n", payload.arg2, 0);
        status = status ? status : VAL_ERROR_POINT(6);
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "\tRXTX_UNMAP failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(7);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "\tfree_rxtx_buffers failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(8);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "\tval_mem_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(9);
    }

    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    return status ? status : (uint32_t)payload.arg3;
}

uint32_t lend_retrieve_mem_access_64_vmsp_client(uint32_t test_run_data)
{
    if (val_get_endpoint_id(VM1) != HYPERVISOR_ID)
        return lend_retrieve_mem_access(test_run_data);
    else
    {
        LOG(TEST, "\tSkipping the check as NS-Hypervisor is absent\n", 0, 0);
        return VAL_SKIP_CHECK;
    }
}

uint32_t lend_retrieve_mem_access_64_vmvm_client(uint32_t test_run_data)
{
    return lend_retrieve_mem_access(test_run_data);
}

uint32_t lend_retrieve_mem_access_64_spsp_client(uint32_t test_run_data)
{
    return lend_retrieve_mem_access(test_run_data);
}
