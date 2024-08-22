/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
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
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
    ffa_endpoint_id_t recipient = val_get_endpoint_id(VM2);
#else
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
#endif
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
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
    mem_region_init.cacheability = 0;
    mem_region_init.shareability = 0;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

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
        LOG(ERROR, "Mem_lend request must return error for invalid sender id %x",
                                                                    payload.arg2);
        status = VAL_ERROR_POINT(2);
    }
    LOG(DBG, "Mem Lend Check for Invalid Sender Complete");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
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
        LOG(ERROR, "Memory allocation failed");
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
    mem_region_init.cacheability = 0;
    mem_region_init.shareability = 0;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

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
        LOG(ERROR, "Lend secure memory to non-secure endpoint must return error %x",
                        payload.arg2);
        status = VAL_ERROR_POINT(5);
    }
    LOG(DBG, "Mem Lend Check for Secure Memory Complete");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(6);
    }

    return status;
}

static uint32_t mem_lend_invalid_total_length_check(void *tx_buf, ffa_endpoint_id_t sender,
                                                     uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
    ffa_endpoint_id_t recipient = val_get_endpoint_id(VM2);
#else
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
#endif
    mem_region_init_t mem_region_init;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
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
    mem_region_init.cacheability = 0;
    mem_region_init.shareability = 0;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

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
        LOG(ERROR, "Mem_lend request must return error for invalid total length err %x",
                        payload.arg2);
        status = VAL_ERROR_POINT(7);
    }
    LOG(DBG, "Mem Lend Check Invalid Memory Length Complete");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(8);
    }

    return status;
}

static uint32_t mem_lend_mem_attribute_check(void *tx_buf, ffa_endpoint_id_t sender,
                                                     uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
    ffa_endpoint_id_t recipient = val_get_endpoint_id(VM2);
#else
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
#endif
    mem_region_init_t mem_region_init;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
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

    if (fid == FFA_MEM_LEND_64)
        val_ffa_mem_lend_64(&payload);
    else
        val_ffa_mem_lend_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "For MEM_LEND, memory type[5:4] must be b'00 err %x", payload.arg2);
        status = VAL_ERROR_POINT(10);
    }
    LOG(DBG, "Mem Lend Check for MBZ complete");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(11);
    }

    return status;
}

static uint32_t mem_lend_instruction_access_check(void *tx_buf, ffa_endpoint_id_t sender,
                                                     uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
    ffa_endpoint_id_t recipient = val_get_endpoint_id(VM2);
#else
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
#endif
    mem_region_init_t mem_region_init;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        return VAL_ERROR_POINT(12);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    mem_region_init.data_access = FFA_DATA_ACCESS_RW;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_X;
    mem_region_init.type = FFA_MEMORY_NOT_SPECIFIED_MEM;
    mem_region_init.cacheability = 0;
    mem_region_init.shareability = 0;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

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
        LOG(ERROR, "For MEM_LEND, instruction acess[3:2] must be b'00 err %x", payload.arg2);
        status = VAL_ERROR_POINT(13);
    }
    LOG(DBG, "Mem Lend Check for Instruction access field complete");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(14);
    }

    return status;
}

static uint32_t mem_lend_invalid_ep_count_check(void *tx_buf, ffa_endpoint_id_t sender,
                                                     uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
    ffa_endpoint_id_t recipient = val_get_endpoint_id(VM2);
#else
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
#endif
    mem_region_init_t mem_region_init;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    struct ffa_memory_region *memory_region;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        return VAL_ERROR_POINT(15);
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
    mem_region_init.cacheability = 0;
    mem_region_init.shareability = 0;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    memory_region = mem_region_init.memory_region;
    /* The Endpoint memory access descriptor count field in the transaction descriptor must be\
     * set to a non-zero value.
     */
    memory_region->receiver_count = 0;
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_LEND_64)
        val_ffa_mem_lend_64(&payload);
    else
        val_ffa_mem_lend_32(&payload);

    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_lend request must return error for invalid endpoint count err %x",
                        payload.arg2);
        status = VAL_ERROR_POINT(16);
    }
    LOG(DBG, "Mem Lend complete");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(17);
    }

    return status;
}

static uint32_t mem_lend_invalid_ep_desc_offset_check(void *tx_buf, ffa_endpoint_id_t sender,
                                                     uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
    ffa_endpoint_id_t recipient = val_get_endpoint_id(VM2);
#else
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
#endif
    mem_region_init_t mem_region_init;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    struct ffa_memory_region *memory_region;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        return VAL_ERROR_POINT(18);
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
    mem_region_init.cacheability = 0;
    mem_region_init.shareability = 0;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    /* The Offset field in the Endpoint memory access descriptor of each Borrower must be set to
     * the offset of the composite memory region descriptor.
     */
    memory_region = mem_region_init.memory_region;
    memory_region->receivers[0].composite_memory_region_offset = 0;
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_LEND_64)
        val_ffa_mem_lend_64(&payload);
    else
        val_ffa_mem_lend_32(&payload);

    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_lend must return error for invalid endpoint descriptor offset err %x",
                        payload.arg2);
        status = VAL_ERROR_POINT(19);
    }
    LOG(DBG, "Mem Lend Check for Invalid EP Desc Offset complete");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(20);
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
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(21);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(22);
        goto free_memory;
    }

    /* Invalid total length check */
    status = mem_lend_invalid_total_length_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Invalid endpoint count check */
    status = mem_lend_invalid_ep_count_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Invalid endpoint descriptor offset check */
    status = mem_lend_invalid_ep_desc_offset_check(mb.send, sender, fid);
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

    /* Owner must not specify the instuction access attributes */
    status = mem_lend_instruction_access_check(mb.send, sender, fid);
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
        LOG(ERROR, "RXTX_UNMAP failed");
        status = status ? status : VAL_ERROR_POINT(23);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "val_mem_free failed");
        status = status ? status : VAL_ERROR_POINT(24);
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
        LOG(TEST, "FFA_MEM_LEND not supported, skipping the check");
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
