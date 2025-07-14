/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define INVALID_ID 0xFFFF

static uint32_t mem_donate_invalid_epid_check(void *tx_buf,
                    ffa_endpoint_id_t sender,
                    ffa_endpoint_id_t receiver,
                    uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint8_t *pages = NULL;
    uint64_t size = 0x1000;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed\n");
        return VAL_ERROR_POINT(1);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    /* Relayer must ensure that the Endpoint ID field in each Memory access permissions descriptor
     * specifies a valid endpoint. The Relayer must return INVALID_PARAMETERS in case of an error.
     */
    mem_region_init.sender = sender;
    mem_region_init.receiver = receiver;
    mem_region_init.memory_region = tx_buf;
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

    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "Mem_donate request must return error for invalid id %x\n", payload.arg2);
        status = VAL_ERROR_POINT(2);
    }

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed\n");
        status = status ? status : VAL_ERROR_POINT(3);
    }

    return status;
}

static uint32_t mem_donate_data_access_perm_check(void *tx_buf, ffa_endpoint_id_t sender,
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
        LOG(ERROR, "Memory allocation failed\n");
        return VAL_ERROR_POINT(4);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    /* If the Receiver is a PE or Proxy endpoint,
     * the Relayer must return INVALID_PARAMETERS if
     * the data access value is not b'00.
     * */
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

    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "For MEM_DONATE, data access[1:0] perm must be b'00\n");
        status = VAL_ERROR_POINT(5);
    }

    LOG(DBG, "Mem Donate Access Perm Check Complete\n");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed\n");
        status = status ? status : VAL_ERROR_POINT(6);
    }
    return status;
}

static uint32_t mem_donate_mem_attribute_check(void *tx_buf, ffa_endpoint_id_t sender, uint32_t fid)
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
        LOG(ERROR, "Memory allocation failed\n");
        return VAL_ERROR_POINT(7);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    /* If the Receiver is a PE or Proxy endpoint,
     * the Relayer must return INVALID_PARAMETERS if
     * the bits[5:4] != b'00..
     * */
    mem_region_init.data_access = FFA_DATA_ACCESS_NOT_SPECIFIED;
    mem_region_init.instruction_access = FFA_INSTRUCTION_ACCESS_NOT_SPECIFIED;
    mem_region_init.type = FFA_MEMORY_NORMAL_MEM;
    mem_region_init.cacheability = FFA_MEMORY_CACHE_NON_CACHEABLE;
    mem_region_init.shareability = FFA_MEMORY_OUTER_SHAREABLE;
    mem_region_init.multi_share = false;
    mem_region_init.receiver_count = 1;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "For MEM_DONATE, memory type[5:4] must be b'00\n");
        status = VAL_ERROR_POINT(8);
    }

    LOG(DBG, "Mem Donate Attribute Error Check Complete\n");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed\n");
        status = status ? status : VAL_ERROR_POINT(9);
    }
    return status;
}

static uint32_t mem_donate_mmio_check(void *tx_buf, ffa_endpoint_id_t sender, uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 1 && PLATFORM_SP_EL == -1)
    ffa_endpoint_id_t recipient = val_get_endpoint_id(VM2);
#else
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP2);
#endif
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    /* Framework does not permit: Access to a device MMIO region to be
     * granted to another partition during run-time.
     */
    if (VAL_IS_ENDPOINT_SECURE(val_get_curr_endpoint_logical_id()))
        constituents[0].address = (void *)PLATFORM_S_UART_BASE;
    else
        constituents[0].address = (void *)PLATFORM_NS_UART_BASE;

    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
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

    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);

    LOG(DBG, "Mem Donate MMIO Check Complete\n");

    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "Framework must not allow to donate mmio region during runtime\n");
        status = VAL_ERROR_POINT(10);
    }

    return status;
}

static uint32_t mem_donate_instruction_access_perm_check(void *tx_buf, ffa_endpoint_id_t sender,
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
        LOG(ERROR, "Memory allocation failed\n");
        return VAL_ERROR_POINT(11);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
    mem_region_init.tag = 0;
    mem_region_init.flags = 0;
    /* If the Receiver is a PE or Proxy endpoint,
     * the Relayer must return INVALID_PARAMETERS if
     * the instruction access value is not b'00.
     * */
    mem_region_init.data_access = FFA_DATA_ACCESS_NOT_SPECIFIED;
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

    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);

    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "For MEM_DONATE, instruction access[3:2] perm must be b'00\n");
        status = VAL_ERROR_POINT(12);
    }

    LOG(DBG, "Mem Donate Instruction Access Check Complete\n");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed\n");
        status = status ? status : VAL_ERROR_POINT(13);
    }

    return status;
}

static uint32_t mem_donate_invalid_ep_count_check(void *tx_buf, ffa_endpoint_id_t sender,
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
    struct ffa_memory_region *memory_region;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed\n");
        return VAL_ERROR_POINT(14);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
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
    memory_region = mem_region_init.memory_region;
    /* The Endpoint memory access descriptor count field in the transaction descriptor
     * must be set to 1
     */
    memory_region->receiver_count = 0;
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);

    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_donate request must return error for invalid endpoint count err %x\n",
                            payload.arg2);
        status = VAL_ERROR_POINT(15);
    }

    LOG(DBG, "Mem Donate Invalid EP Check Complete\n");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed\n");
        status = status ? status : VAL_ERROR_POINT(16);
    }

    return status;
}

static uint32_t mem_donate_invalid_ep_desc_offset_check(void *tx_buf, ffa_endpoint_id_t sender,
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
    struct ffa_memory_region *memory_region;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    pages = (uint8_t *)val_memory_alloc(size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed\n");
        return VAL_ERROR_POINT(17);
    }

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

    mem_region_init.memory_region = tx_buf;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
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
    /* The Offset field of the Endpoint memory access descriptor must be set to the offset of
     * the composite memory region descriptor.
     */
    memory_region = mem_region_init.memory_region;
    memory_region->receivers[0].composite_memory_region_offset = 0;
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_DONATE_64)
        val_ffa_mem_donate_64(&payload);
    else
        val_ffa_mem_donate_32(&payload);

    if (payload.fid != FFA_ERROR_32)
    {
        LOG(ERROR, "MEM_DONATE must return error for invalid endpoint descriptor offset err %x\n",
                                    payload.arg2);
        status = VAL_ERROR_POINT(18);
    }

    LOG(DBG, "Mem Donate EP Desc Offset Check Complete\n");

    if (val_memory_free(pages, size))
    {
        LOG(ERROR, "val_mem_free failed\n");
        status = status ? status : VAL_ERROR_POINT(19);
    }

    return status;
}

static uint32_t ffa_mem_donate_helper(uint32_t test_run_data, uint32_t fid)
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
        LOG(ERROR, "Failed to allocate RxTx buffer\n");
        status = VAL_ERROR_POINT(20);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed\n");
        status = VAL_ERROR_POINT(21);
        goto free_memory;
    }

    /* Can't donate mmio check */
    status = mem_donate_mmio_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Owner must not specify the data access permission */
    status = mem_donate_data_access_perm_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Owner must not specify the instruction access permission */
    status = mem_donate_instruction_access_perm_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Invalid endpoint descriptor offset check */
    status = mem_donate_invalid_ep_desc_offset_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Invalid endpoint count check */
    status = mem_donate_invalid_ep_count_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Owner must not specify the memory attributes */
    status = mem_donate_mem_attribute_check(mb.send, sender, fid);
    if (status)
        goto rxtx_unmap;

    /* Invalid receiver id check */
    status = mem_donate_invalid_epid_check(mb.send, sender, INVALID_ID, fid);
    if (status)
        goto rxtx_unmap;

    /* donate mem to self check */
    status = mem_donate_invalid_epid_check(mb.send, sender, sender, fid);
    if (status)
        goto rxtx_unmap;

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed\n");
        status = status ? status : VAL_ERROR_POINT(22);
    }

free_memory:
    if (val_memory_free(mb.recv, size) || val_memory_free(mb.send, size))
    {
        LOG(ERROR, "val_mem_free failed\n");
        status = status ? status : VAL_ERROR_POINT(23);
    }

    return status;
}

uint32_t donate_input_error_checks_client(uint32_t test_run_data)
{
    uint32_t status_32, status_64, status;

    status_64 = val_is_ffa_feature_supported(FFA_MEM_DONATE_64);
    status_32 = val_is_ffa_feature_supported(FFA_MEM_DONATE_32);
    if (status_64 && status_32)
    {
        LOG(TEST, "FFA_MEM_DONATE not supported, skipping the check\n");
        return VAL_SKIP_CHECK;
    }
    else if (status_64 && !status_32)
    {
        status = ffa_mem_donate_helper(test_run_data, FFA_MEM_DONATE_32);
    }
    else if (!status_64 && status_32)
    {
        status = ffa_mem_donate_helper(test_run_data, FFA_MEM_DONATE_64);
    }
    else
    {
        status = ffa_mem_donate_helper(test_run_data, FFA_MEM_DONATE_64);
        if (status)
            return status;

        status = ffa_mem_donate_helper(test_run_data, FFA_MEM_DONATE_32);
        if (status)
            return status;
    }
    return status;
}
