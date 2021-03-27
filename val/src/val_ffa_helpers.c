/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_interfaces.h"

/**
 * Initialises the header of the given `ffa_memory_region`, not including the
 * composite memory region offset.
 */
static void ffa_memory_region_init_header(mem_region_init_t *mem_region_init,
                    ffa_memory_attributes_t attributes,
                    ffa_memory_handle_t handle,
                    ffa_memory_access_permissions_t permissions)
{
    struct ffa_memory_region *memory_region = mem_region_init->memory_region;

    memory_region->sender = mem_region_init->sender;
    memory_region->attributes = attributes;
    memory_region->reserved_0 = 0;
    memory_region->flags = mem_region_init->flags;
    memory_region->handle = handle;
    memory_region->tag = mem_region_init->tag;
    memory_region->reserved_1 = 0;
    memory_region->receiver_count = 1;
    memory_region->receivers[0].receiver_permissions.receiver = mem_region_init->receiver;
    memory_region->receivers[0].receiver_permissions.permissions = permissions;
    memory_region->receivers[0].receiver_permissions.flags = 0;
    memory_region->receivers[0].reserved_0 = 0;
}

/**
 * Initialises the given `ffa_memory_region` and copies as many as possible of
 * the given constituents to it.
 *
 * Returns the number of constituents remaining which wouldn't fit, and (via
 * return parameters) the size in bytes of the first fragment of data copied to
 * `memory_region` (attributes, constituents and memory region header size), and
 * the total size of the memory sharing message including all constituents.
 */
uint32_t val_ffa_memory_region_init(mem_region_init_t *mem_region_init,
                 const struct ffa_memory_region_constituent constituents[],
                 uint32_t constituent_count)
{
    ffa_memory_access_permissions_t permissions = 0;
    ffa_memory_attributes_t attributes = 0;
    struct ffa_composite_memory_region *composite_memory_region;
    uint32_t fragment_max_constituents;
    uint32_t count_to_copy;
    uint32_t i;
    uint32_t constituents_offset;

    /* Set memory region's permissions. */
    ffa_set_data_access_attr(&permissions, mem_region_init->data_access);
    ffa_set_instruction_access_attr(&permissions, mem_region_init->instruction_access);

    /* Set memory region's page attributes. */
    ffa_set_memory_type_attr(&attributes, mem_region_init->type);
    ffa_set_memory_cacheability_attr(&attributes, mem_region_init->cacheability);
    ffa_set_memory_shareability_attr(&attributes, mem_region_init->shareability);

    ffa_memory_region_init_header(mem_region_init, attributes, 0, permissions);

    /*
     * Note that `sizeof(struct_ffa_memory_region)` and `sizeof(struct
     * ffa_memory_access)` must both be multiples of 16 (as verified by the
     * asserts in `ffa_memory.c`, so it is guaranteed that the offset we
     * calculate here is aligned to a 64-bit boundary and so 64-bit values
     * can be copied without alignment faults.
     */
    mem_region_init->memory_region->receivers[0].composite_memory_region_offset =
        (uint32_t)(sizeof(struct ffa_memory_region) +
        mem_region_init->memory_region->receiver_count *
            sizeof(struct ffa_memory_access));

    composite_memory_region =
        ffa_memory_region_get_composite(mem_region_init->memory_region, 0);
    composite_memory_region->page_count = 0;
    composite_memory_region->constituent_count = constituent_count;
    composite_memory_region->reserved_0 = 0;

    constituents_offset =
        (uint32_t)(mem_region_init->memory_region->receivers[0].composite_memory_region_offset +
        sizeof(struct ffa_composite_memory_region));
    fragment_max_constituents =
        (uint32_t)((mem_region_init->memory_region_max_size - constituents_offset) /
        sizeof(struct ffa_memory_region_constituent));

    count_to_copy = constituent_count;
    if (count_to_copy > fragment_max_constituents)
    {
        count_to_copy = fragment_max_constituents;
    }

    for (i = 0; i < constituent_count; ++i)
    {
        if (i < count_to_copy)
        {
            composite_memory_region->constituents[i] =
                constituents[i];
        }
        composite_memory_region->page_count +=
            constituents[i].page_count;
    }

    mem_region_init->total_length =
        (uint32_t)(constituents_offset +
        composite_memory_region->constituent_count *
            sizeof(struct ffa_memory_region_constituent));
    mem_region_init->fragment_length =
        (uint32_t)(constituents_offset +
        count_to_copy *
            sizeof(struct ffa_memory_region_constituent));

    return composite_memory_region->constituent_count - count_to_copy;
}

/**
 * Initialises the given `ffa_memory_region` to be used for an
 * `FFA_MEM_RETRIEVE_REQ` by the receiver of a memory transaction.
 *
 * Returns the size of the message written.
 */
uint32_t val_ffa_memory_retrieve_request_init(mem_region_init_t *mem_region_init,
                                ffa_memory_handle_t handle)
{
    ffa_memory_access_permissions_t permissions = 0;
    ffa_memory_attributes_t attributes = 0;

    /* Set memory region's permissions. */
    ffa_set_data_access_attr(&permissions, mem_region_init->data_access);
    ffa_set_instruction_access_attr(&permissions, mem_region_init->instruction_access);

    /* Set memory region's page attributes. */
    ffa_set_memory_type_attr(&attributes, mem_region_init->type);
    ffa_set_memory_cacheability_attr(&attributes, mem_region_init->cacheability);
    ffa_set_memory_shareability_attr(&attributes, mem_region_init->shareability);

    ffa_memory_region_init_header(mem_region_init, attributes, handle, permissions);

    /*
     * Offset 0 in this case means that the hypervisor should allocate the
     * address ranges. This is the only configuration supported by Hafnium,
     * as it enforces 1:1 mappings in the stage 2 page tables.
     */
    mem_region_init->memory_region->receivers[0].composite_memory_region_offset = 0;
    mem_region_init->memory_region->receivers[0].reserved_0 = 0;

    return (uint32_t)(sizeof(struct ffa_memory_region) +
           mem_region_init->memory_region->receiver_count * sizeof(struct ffa_memory_access));
}

/**
 * @brief - Returns FF-A interface is implemented or not.
 * @param fid - FF-A instance fid.
 * @return - Returns success/error status code.
**/
uint32_t val_is_ffa_feature_supported(uint32_t fid)
{
    ffa_args_t payload;

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = fid;
    val_ffa_features(&payload);
    if (payload.fid == FFA_ERROR_32)
        return VAL_ERROR;
    else
        return VAL_SUCCESS;
}