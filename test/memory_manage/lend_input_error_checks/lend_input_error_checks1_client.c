/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t mem_lend_invalid_sender_id_check(void *tx_buf, ffa_endpoint_id_t sender,
                    uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "\tMemory allocation failed\n", 0, 0);
        return VAL_ERROR_POINT(1);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    /* Relayer must validate the Sender endpoint ID field in the transaction descriptor to ensure
     * that the Lender is the Owner of the memory region and a PE endpoint.
     * Must return DENIED in case of an error.
     */
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.memory_region = tx_buf;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NOT_SPECIFIED_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
    mem_region_init.multi_share = false;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_LEND_64)
        val_ffa_mem_lend_64(&payload);
    else
        val_ffa_mem_lend_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "\tMem_lend request must return error for invalid sender id %x\n",
                                                                    payload.arg2, 0);
        status = VAL_ERROR_POINT(2);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "\tval_mem_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(3);
    }

    return status;
}

static uint32_t mem_lend_sp_to_ns_check(void *tx_buf, ffa_endpoint_id_t sender, uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t recipient = val_get_endpoint_id(VM1);
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "\tMemory allocation failed\n", 0, 0);
        return VAL_ERROR_POINT(4);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.memory_region = tx_buf;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NOT_SPECIFIED_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
    mem_region_init.multi_share = false;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_LEND_64)
        val_ffa_mem_lend_64(&payload);
    else
        val_ffa_mem_lend_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "\tLend secure memory to non-secure endpoint must return error %x\n",
                        payload.arg2, 0);
        status = VAL_ERROR_POINT(5);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "\tval_mem_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(6);
    }

    return status;
}

static uint32_t mem_lend_invalid_total_length_check(void *tx_buf, ffa_endpoint_id_t sender,
                                                     uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
    mem_region_init_t mem_region_init;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "\tMemory allocation failed\n", 0, 0);
        return VAL_ERROR_POINT(7);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NOT_SPECIFIED_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
    mem_region_init.multi_share = false;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    /* Pass invalid total length parameter to MEM_LEND and check for error status code */
    payload.arg1 = mem_region_init.total_length + 0x10;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_LEND_64)
        val_ffa_mem_lend_64(&payload);
    else
        val_ffa_mem_lend_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tMem_lend request must return error for invalid total length err %x\n",
                        payload.arg2, 0);
        status = VAL_ERROR_POINT(7);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "\tval_mem_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(8);
    }

    return status;
}

static uint32_t mem_lend_mem_attribute_check(void *tx_buf, ffa_endpoint_id_t sender,
                                                     uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
    mem_region_init_t mem_region_init;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "\tMemory allocation failed\n", 0, 0);
        return VAL_ERROR_POINT(9);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_WRITE_BACK;
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
    mem_region_init.multi_share = false;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_LEND_64)
        val_ffa_mem_lend_64(&payload);
    else
        val_ffa_mem_lend_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tFor MEM_LEND, memory type must be b'00 err %x\n", payload.arg2, 0);
        status = VAL_ERROR_POINT(10);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "\tval_mem_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(11);
    }

    return status;
}

static uint32_t ffa_mem_lend_helper(uint32_t test_run_data, uint32_t fid)
{
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    mb_buf_t mb;
    uint64_t size = 0x1000;

    mb.send = val_memory_alloc(size);
    mb.recv = val_memory_alloc(size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        status = VAL_ERROR_POINT(12);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "\tRxTx Map failed\n", 0, 0);
        status = VAL_ERROR_POINT(13);
        goto free_memory;
    }

    /* Invalid total length check */
    status = mem_lend_invalid_total_length_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Invalid sender id check */
    status = mem_lend_invalid_sender_id_check(mb.send, val_get_endpoint_id(SP2), fid);
    if (status)
        goto rxtx_unmap;

    /* Owner must not specify the memory attributes */
    status = mem_lend_mem_attribute_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Relayer must ensure that a request by a SP to lend Secure memory to a NS-Endpoint is
     * rejected by returning the DENIED error code.
     */
    if (VAL_IS_ENDPOINT_SECURE(val_get_curr_endpoint_logical_id()))
    {
        status = mem_lend_sp_to_ns_check(mb.send, sender, fid);
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "\tRXTX_UNMAP failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(14);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "\tval_mem_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(15);
    }

    return status;
}

uint32_t lend_input_error_checks1_client(uint32_t test_run_data)
{
    uint32_t status_32, status_64, status;

    status_64 = val_is_ffa_feature_supported(FFA_MEM_LEND_64);
    status_32 = val_is_ffa_feature_supported(FFA_MEM_LEND_32);
    if (status_64 && status_32)
    {
        LOG(TEST, "\tFFA_MEM_LEND not supported, skipping the check\n", 0, 0);
        return VAL_SKIP_CHECK;
    }
    else if (status_64 && !status_32)
    {
        status = ffa_mem_lend_helper(test_run_data, FFA_MEM_LEND_32);
    }
    else if (!status_64 && status_32)
    {
        status = ffa_mem_lend_helper(test_run_data, FFA_MEM_LEND_64);
    }
    else
    {
        status = ffa_mem_lend_helper(test_run_data, FFA_MEM_LEND_64);
        if (status)
            return status;

        status = ffa_mem_lend_helper(test_run_data, FFA_MEM_LEND_32);
        if (status)
            return status;
    }

    return status;
}
