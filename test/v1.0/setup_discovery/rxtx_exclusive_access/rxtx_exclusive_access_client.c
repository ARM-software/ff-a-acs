/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t rxtx_exclusive_access_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint64_t size = PAGE_SIZE_4K;
    void *rx_buff, *tx_buff;
    uint64_t rx_ipa, tx_ipa;

    tx_buff = val_aligned_alloc(PAGE_SIZE_4K, size);
    rx_buff = val_aligned_alloc(PAGE_SIZE_4K, size);
    if (rx_buff == NULL || tx_buff == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)tx_buff, (uint64_t)rx_buff, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    tx_ipa = (uint64_t)val_mem_virt_to_phys((void *)tx_buff);
    rx_ipa = (uint64_t)val_mem_virt_to_phys((void *)rx_buff);

    LOG(DBG, "tx_ipa %x rx_ipa %x", tx_ipa, rx_ipa);

    /* Both Hypervisor and SPM must ensure the caller has
     * exclusive access and ownership of the RX/TX
     * buffer memory regions.
     * - Share client rxtx buffer addresses with server.
     * - Server to try mapping client rxtx buffers and
     *   expect error code
     */
    val_select_server_fn_direct(test_run_data,
                                tx_ipa & 0xffffffff,
                                (uint32_t)((tx_ipa & 0xffffffff00000000) >> 32),
                                rx_ipa & 0xffffffff,
                                (uint32_t)((rx_ipa & 0xffffffff00000000) >> 32));

    if (val_rxtx_unmap(val_get_curr_endpoint_id()))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = VAL_ERROR_POINT(3);
    }

free_memory:
    if (val_free(rx_buff) || val_free(tx_buff))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(4);
    }

    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    return status ? status : (uint32_t)payload.arg3;
}
