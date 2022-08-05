/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

static uint32_t test_rxtx_unmap_before_map(void)
{
    ffa_args_t payload;

    /* INVALID_PARAMETERS: There is no buffer pair registered on behalf of the caller.
     * Call FFA_RXTX_UNMAP API with registering buffers with partition manager.
     * */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)(val_get_curr_endpoint_id() << 16);
    val_ffa_rxtx_unmap(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tCheck failed for rxtx_unmap-1: fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        return VAL_ERROR_POINT(1);
    }

   return VAL_SUCCESS;
}

static uint32_t test_rxtx_map_unaligned_buffers(void)
{
    ffa_args_t payload;
    void *rx_buff, *tx_buff;
    uint64_t size = PAGE_SIZE_4K;
    uint32_t status = VAL_SUCCESS;

    tx_buff = val_memory_alloc(size);
    rx_buff = val_memory_alloc(size);
    if (rx_buff == NULL || tx_buff == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        return VAL_ERROR_POINT(2);
    }

    /* Map TX and RX buffers with TX unaligned to max translation granule size */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint64_t)val_mem_virt_to_phys((void *)tx_buff) + 0x100;
    payload.arg2 = (uint64_t)val_mem_virt_to_phys((void *)rx_buff);
    payload.arg3 = (uint32_t)(size/PAGE_SIZE_4K);
    val_ffa_rxtx_map_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tCheck failed for rxtx_map-1: fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(3);
        goto free_memory1;
    }

    /* Map TX and RX buffers with RX unaligned to max translation granule size */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint64_t)val_mem_virt_to_phys((void *)tx_buff);
    payload.arg2 = (uint64_t)val_mem_virt_to_phys((void *)rx_buff) + 0x100;
    payload.arg3 = (uint32_t)(size/PAGE_SIZE_4K);
    val_ffa_rxtx_map_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tCheck failed for rxtx_map-2: fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(4);
        goto free_memory1;
    }

free_memory1:
    if (val_memory_free(rx_buff, size) || val_memory_free(tx_buff, size))
    {
        LOG(ERROR, "\tval_memory_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(5);
    }

    return status;
}

static uint32_t test_rxtx_map_invalid_buffer_size(void)
{
    ffa_args_t payload;
    void *rx_buff, *tx_buff;
    uint64_t size = PAGE_SIZE_4K, alignment_boundary;
    uint32_t status = VAL_SUCCESS;

    tx_buff = val_memory_alloc(size);
    rx_buff = val_memory_alloc(size);
    if (rx_buff == NULL || tx_buff == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        return VAL_ERROR_POINT(6);
    }

    /* Check RXTX_MAP alignment boundary w2[1:0] */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = FFA_RXTX_MAP_64;
    val_ffa_features(&payload);

    alignment_boundary = VAL_EXTRACT_BITS(payload.arg2, 0, 1);
    if (alignment_boundary != FFA_RXTX_MAP_4K_SIZE)
    {
         /* Pass page_count not equal to multiple of max translation granule size */
         val_memset(&payload, 0, sizeof(ffa_args_t));
         payload.arg1 = (uint64_t)val_mem_virt_to_phys((void *)tx_buff);
         payload.arg2 = (uint64_t)val_mem_virt_to_phys((void *)rx_buff);
         payload.arg3 = 1;
         val_ffa_rxtx_map_64(&payload);
         if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
         {
             LOG(ERROR, "\tCheck failed for rxtx_map-3: fid=0x%x, err=0x%x\n",
                 payload.fid, payload.arg2);
             status = VAL_ERROR_POINT(7);
             goto free_memory2;
         }
    }

    /* Try to map using zero buffer size */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint64_t)val_mem_virt_to_phys((void *)tx_buff);
    payload.arg2 = (uint64_t)val_mem_virt_to_phys((void *)rx_buff);
    payload.arg3 = 0;
    val_ffa_rxtx_map_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_INVALID_PARAMETERS))
    {
        LOG(ERROR, "\tCheck failed for rxtx_map-3: fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(8);
        goto free_memory2;
    }

free_memory2:
    if (val_memory_free(rx_buff, size) || val_memory_free(tx_buff, size))
    {
        LOG(ERROR, "\tval_memory_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(9);
    }

    return status;
}

static uint32_t test_rxtx_map_denied(void)
{
    ffa_args_t payload;
    void *rx_buff, *tx_buff;
    uint64_t size = PAGE_SIZE_4K;
    uint32_t status = VAL_SUCCESS;

    tx_buff = val_memory_alloc(size);
    rx_buff = val_memory_alloc(size);
    if (rx_buff == NULL || tx_buff == NULL)
    {
        LOG(ERROR, "\tFailed to allocate RxTx buffer\n", 0, 0);
        return VAL_ERROR_POINT(10);
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)tx_buff, (uint64_t)rx_buff, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "\tRxTx Map failed\n", 0, 0);
        status = VAL_ERROR_POINT(11);
        goto free_memory3;
    }

    /* Try to register the buffers again */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint64_t)val_mem_virt_to_phys((void *)tx_buff);
    payload.arg2 = (uint64_t)val_mem_virt_to_phys((void *)rx_buff);
    payload.arg3 = 1;
    val_ffa_rxtx_map_64(&payload);
    if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
    {
        LOG(ERROR, "\tCheck failed for rxtx_map-4: fid=0x%x, err=0x%x\n",
            payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(12);
        goto free_memory3;
    }

    /* Unmap buffers with correct endpoint id */
    if (val_rxtx_unmap(val_get_curr_endpoint_id()))
    {
        LOG(ERROR, "\tval_rxtx_unmap failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(13);
    }

free_memory3:
    if (val_memory_free(rx_buff, size) || val_memory_free(tx_buff, size))
    {
        LOG(ERROR, "\tval_memory_free failed\n", 0, 0);
        status = status ? status : VAL_ERROR_POINT(14);
    }

    return status;
}

uint32_t ffa_rxtx_map_and_unmap_client(uint32_t test_run_data)
{
    uint32_t status = VAL_SUCCESS;

    status = test_rxtx_map_unaligned_buffers();
    if (status)
        return status;

    status = test_rxtx_map_invalid_buffer_size();
    if (status)
        return status;

    status = test_rxtx_map_denied();
    if (status)
        return status;

    status = test_rxtx_unmap_before_map();
    if (status)
        return status;

    /* Unused argument */
    (void)test_run_data;
    return status;
}
