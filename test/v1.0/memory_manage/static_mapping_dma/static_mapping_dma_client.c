/*
 * Copyright (c) 2025-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"
#include "pal_smmuv3_testengine.h"

static uint32_t ffa_mem_share_helper(uint32_t test_run_data, uint32_t fid)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP3);
    mb_buf_t mb;
    memory_region_descriptor_t mem_desc;
    uint64_t size = 0x1000;
    ffa_memory_region_flags_t flags = 0;
    ffa_memory_handle_t handle;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
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

    /* Map the region into endpoint translation */
    mem_desc.virtual_address = PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION;
    mem_desc.physical_address = PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION;
    mem_desc.length = size * 2;
    if (VAL_IS_ENDPOINT_SECURE(client_logical_id))
        mem_desc.attributes = ATTR_RW_DATA;
    else
        mem_desc.attributes = ATTR_RW_DATA | ATTR_NS;

    if (val_mem_map_pgt(&mem_desc))
    {
        LOG(ERROR, "Va to pa mapping failed\n");
        status =  VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

    /* Map the region into endpoint translation */
    mem_desc.virtual_address = PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID;
    mem_desc.physical_address = PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID;
    mem_desc.length = size * 2;
    if (VAL_IS_ENDPOINT_SECURE(client_logical_id))
        mem_desc.attributes = ATTR_RW_DATA;
    else
        mem_desc.attributes = ATTR_RW_DATA | ATTR_NS;

    if (val_mem_map_pgt(&mem_desc))
    {
        LOG(ERROR, "\tVa to pa mapping failed\n", 0, 0);
        status =  VAL_ERROR_POINT(3);
        goto rxtx_unmap;
    }

    val_memset((void *)PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION, 0xab, size);
    val_memset((void *)PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID, 0xea, size);

#ifndef TARGET_LINUX
    /* Initiate the DMA transactions to
     * memory regions using device upstream of SMMU
     */
    flush_dcache_range(PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION, size*2);
    flush_dcache_range(PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID, size*2);
#endif

    if (VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(sender)))
    {
        smmuv3_configure_testengine(PLATFORM_SMMU_STREAM_ID,
                 PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION,
                 PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION + PAGE_SIZE_4K, size, true);
    }
    else
    {
        smmuv3_configure_testengine(PLATFORM_SMMU_STREAM_ID,
                 PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION,
                 PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION + PAGE_SIZE_4K, size, false);
    }

    if (VAL_IS_ENDPOINT_SECURE(val_get_endpoint_logical_id(sender)))
    {
        smmuv3_configure_testengine(PLATFORM_SMMU_STREAM_ID,
                 PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID,
                 PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID + PAGE_SIZE_4K, size, true);
    }
    else
    {
        smmuv3_configure_testengine(PLATFORM_SMMU_STREAM_ID,
                 PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID,
                 PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID + PAGE_SIZE_4K, size, false);
    }

    if (val_memcmp((void *)PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION,
                    (void *)PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION + PAGE_SIZE_4K, size))
    {
        LOG(ERROR, "Data mismatch\n");
        status =  VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }

    if (!(val_memcmp((void *)PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID,
                    (void *)PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION_INVALID + PAGE_SIZE_4K, size)))
    {
        LOG(ERROR, "\tDMA transaction should not have occured\n", 0, 0);
        status =  VAL_ERROR_POINT(5);
        goto rxtx_unmap;
    }

    val_memset((void *)PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION + (2*PAGE_SIZE_4K), 0x0, size);

    constituents[0].address = (void *)PLAT_SMMU_UPSTREAM_DEVICE_MEM_REGION;
    constituents[0].page_count = 2;

    val_memset(&mem_region_init, 0x0, sizeof(mem_region_init));
    mem_region_init.memory_region = mb.send;
    mem_region_init.sender = sender;
    mem_region_init.receiver = recipient;
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

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;

    if (fid == FFA_MEM_SHARE_64)
        val_ffa_mem_share_64(&payload);
    else
        val_ffa_mem_share_32(&payload);

    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_share request must fail for static dma mappings fid:%x err:%x\n",
                                                                payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(6);
        goto reclaim;
    }
    LOG(DBG, "Mem Share Complete\n");

reclaim:
    handle = ffa_mem_success_handle(payload);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)handle;
    payload.arg2 = (uint32_t)(handle >> 32);
    payload.arg3 = 0;
    val_ffa_mem_reclaim(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem Reclaim failed err %x\n", payload.arg2);
        status = VAL_ERROR_POINT(6);
    }
    LOG(DBG, "Mem Reclaim Complete\n");

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed\n");
        status = status ? status : VAL_ERROR_POINT(7);
    }

free_memory:
    if (val_free(mb.recv) || val_free(mb.send))
    {
        LOG(ERROR, "free_rxtx_buffers failed\n");
        status = status ? status : VAL_ERROR_POINT(8);
    }

    return status;
}

uint32_t static_mapping_dma_client(uint32_t test_run_data)
{
    uint32_t status_32, status_64, status;

    status_64 = val_is_ffa_feature_supported(FFA_MEM_SHARE_64);
    status_32 = val_is_ffa_feature_supported(FFA_MEM_SHARE_32);
    if (status_64 && status_32)
    {
        LOG(TEST, "FFA_MEM_SHARE not supported, skipping the check\n");
        return VAL_SKIP_CHECK;
    }
    else if (status_64 && !status_32)
    {
        status = ffa_mem_share_helper(test_run_data, FFA_MEM_SHARE_32);
    }
    else if (!status_64 && status_32)
    {
        status = ffa_mem_share_helper(test_run_data, FFA_MEM_SHARE_64);
    }
    else
    {
        status = ffa_mem_share_helper(test_run_data, FFA_MEM_SHARE_64);
        if (status)
            return status;

        status = ffa_mem_share_helper(test_run_data, FFA_MEM_SHARE_32);
        if (status)
            return status;
    }
    return status;
}