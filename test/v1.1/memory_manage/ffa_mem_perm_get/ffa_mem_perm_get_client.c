/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

uint32_t ffa_mem_perm_get_client(uint32_t test_run_data)
{
    uint32_t status = VAL_SUCCESS;
    uint32_t size = 0x1000;
    ffa_args_t payload;
    ffa_endpoint_id_t curr_ep_logical_id;
    uint8_t *pages = NULL;

    pages = (uint8_t *)val_aligned_alloc(PAGE_SIZE_4K, size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        status = VAL_ERROR_POINT(1);
        goto free_memory;
    }

    curr_ep_logical_id = val_get_curr_endpoint_logical_id();

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint64_t)pages;
    val_ffa_mem_perm_get_32(&payload);
    if (curr_ep_logical_id == VM1)
    {
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_NOT_SUPPORTED))
        {
            LOG(ERROR, "ffa_mem_perm_set must return error for invalid instance %x",
                            payload.arg2);
            status = VAL_ERROR_POINT(2);
            goto free_memory;
        }
    }
    else if (curr_ep_logical_id == SP1)
    {
        if ((payload.fid != FFA_ERROR_32) || (payload.arg2 != FFA_ERROR_DENIED))
        {
            LOG(ERROR, "ffa_mem_perm_set must return error for post-initialization %x",
                            payload.arg2);
            status = VAL_ERROR_POINT(3);
            goto free_memory;
        }
    }

free_memory:
   if (val_free(pages))
    {
        LOG(ERROR, "val_free failed");
        status = status ? status : VAL_ERROR_POINT(4);
    }

    /* Unused argument */
    (void)test_run_data;
    return status;
}
