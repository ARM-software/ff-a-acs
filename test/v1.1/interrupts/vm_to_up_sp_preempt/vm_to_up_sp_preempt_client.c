/*
 * Copyright (c) 2024-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "test_database.h"

#define IRQ_TRIGGERED 0xABCDABCD
#define NS_WD_TIMEOUT 500000U
static uint32_t *pages;

static int wd_irq_handler(void)
{
    *(volatile uint32_t *)pages = (uint32_t)IRQ_TRIGGERED;
    val_ns_wdog_disable();
    LOG(DBG, "NS-WD IRQ Handler Processed");
    return 0;
}

static event_t cpu_booted;
static event_t ffa_run_sec_pe;
static event_t mem_reclaim;

uint32_t vm_to_up_sp_preempt_sec_cpu_client(uint32_t test_num)
{
    ffa_args_t payload;
    ffa_endpoint_id_t recipient = val_get_endpoint_id(SP4);
    uint64_t mpid = val_read_mpidr() & MPID_MASK;
    (void)(test_num);

    /* Tell the boot CPU that the calling CPU has completed the boot */
    LOG(DBG, "Secondary cpu with mpid 0x%x booted", mpid);
    val_send_event(&cpu_booted);

    /* Wait for event from boot cpu */
    val_wait_for_event(&ffa_run_sec_pe);

    /* Schedule the preempted SP using FFA_RUN from Sec CPU*/

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)recipient << 16;
    payload.arg2 = 0;
    val_ffa_run(&payload);
    if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_64)
    {
        LOG(ERROR, "DIRECT_RESP_64 not received");
        return VAL_ERROR_POINT(1);
    }

    /* Tell the boot CPU that the calling CPU has completed the test */
    val_send_event(&mem_reclaim);

    return VAL_SUCCESS;
}

uint32_t vm_to_up_sp_preempt_client(uint32_t test_run_data)
{
    ffa_args_t payload;
    uint32_t status = VAL_SUCCESS;
    uint32_t i, total_cpus = val_get_no_of_cpus(), ret;
    uint64_t boot_mpid, mpid = 0;
    uint32_t client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);
    ffa_endpoint_id_t sender = val_get_endpoint_id(client_logical_id);
    ffa_endpoint_id_t recipient = val_get_endpoint_id(server_logical_id);
    mb_buf_t mb;
    uint64_t size = 0x1000;
    ffa_memory_region_flags_t flags = 0;
    ffa_memory_handle_t handle;
    mem_region_init_t mem_region_init;
    struct ffa_memory_region_constituent constituents[1];
    const uint32_t constituents_count = sizeof(constituents) /
                sizeof(struct ffa_memory_region_constituent);

    if (val_is_ffa_feature_supported(FFA_MEM_SHARE_32))
    {
        LOG(TEST, "FFA_MEM_SHARE_32 not supported, skipping the test");
        return VAL_SKIP_CHECK;
    }

    mb.send = val_aligned_alloc(PAGE_SIZE_4K, size);
    mb.recv = val_aligned_alloc(PAGE_SIZE_4K, size);
    if (mb.send == NULL || mb.recv == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer");
        status = VAL_ERROR_POINT(2);
        goto free_memory;
    }

    /* Map TX and RX buffers */
    if (val_rxtx_map_64((uint64_t)mb.send, (uint64_t)mb.recv, (uint32_t)(size/PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed");
        status = VAL_ERROR_POINT(3);
        goto free_memory;
    }

    pages = (uint32_t *)val_aligned_alloc(PAGE_SIZE_4K, size);
    if (!pages)
    {
        LOG(ERROR, "Memory allocation failed");
        status = VAL_ERROR_POINT(4);
        goto rxtx_unmap;
    }
    val_memset(pages, 0, size);

    /* Setup another VM1  execution context */
    boot_mpid = val_read_mpidr() & MPID_MASK;

    val_init_event(&cpu_booted);
    val_init_event(&mem_reclaim);
    val_init_event(&ffa_run_sec_pe);

    LOG(DBG, "boot cpu mpid %x", boot_mpid, 0);
    for (i = 0; i < total_cpus; i++)
    {
        mpid = val_get_mpid(i);

        LOG(DBG, "Power up secondary CPUs mpid=%x", mpid);
        if (mpid == boot_mpid)
        {
            continue;
        }

        ret = val_power_on_cpu(i);
        if (ret != 0)
        {
            LOG(ERROR, "val_power_on_cpu mpid 0x%x returns %x", mpid, ret);
            return VAL_ERROR_POINT(5);
        }
        break;
    }

    LOG(DBG, "Waiting secondary CPU to turn on ...");
    val_wait_for_event(&cpu_booted);

    /* Schedule SP for setup */
    val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    constituents[0].address = val_mem_virt_to_phys((void *)pages);
    constituents[0].page_count = 1;

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
    mem_region_init.receiver_count = 1;

    val_ffa_memory_region_init(&mem_region_init, constituents, constituents_count);
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = mem_region_init.total_length;
    payload.arg2 = mem_region_init.fragment_length;
    val_ffa_mem_share_32(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem_share request failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(6);
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
        LOG(ERROR, "Direct request failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(7);
        goto rxtx_unmap;
    }

    if (val_irq_register_handler(PLATFORM_NS_WD_INTR, wd_irq_handler))
    {
        LOG(ERROR, "WD interrupt register failed");
        status = VAL_ERROR_POINT(8);
        goto rxtx_unmap;
    }

    val_irq_enable(PLATFORM_NS_WD_INTR, 0);
    val_ns_wdog_enable(NS_WD_TIMEOUT);
    LOG(DBG, "NS-WD IRQ Registered");

    /* Call SP and Check for FFA_INTERRUPT on response */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 =  ((uint32_t)sender << 16) | recipient;
    val_ffa_msg_send_direct_req_64(&payload);
    if (payload.fid != FFA_INTERRUPT_32)
    {
        LOG(ERROR, "FFA_INTERRUPT_32 not received fid %x err %x", payload.fid, payload.arg2);
        status = VAL_ERROR_POINT(9);
        goto free_interrupt;
    }

    /* Interrupt handler should have been triggered */
    if ((*(volatile uint32_t *)pages != IRQ_TRIGGERED))
    {
        LOG(ERROR, "WD interrupt not triggered");
        status =  VAL_ERROR_POINT(10);
        goto free_interrupt;
    }

    val_irq_disable(PLATFORM_NS_WD_INTR);
    LOG(DBG, "NS-WD IRQ Disabled, Call FFA_RUN");

    /* Migrate Client flow to Sec CPU */
    val_send_event(&ffa_run_sec_pe);

    /* Wait for event from Sec CPU */
    val_wait_for_event(&mem_reclaim);

    /* Reclaim Memory and Clean Up */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = (uint32_t)handle;
    payload.arg2 = (uint32_t)(handle >> 32);
    payload.arg3 = 0;
    val_ffa_mem_reclaim(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Mem Reclaim failed err %d", payload.arg2);
        status = VAL_ERROR_POINT(11);
    }
    LOG(DBG, "FFA MEM Reclaim Complete");

free_interrupt:
    if (val_irq_unregister_handler(PLATFORM_NS_WD_INTR))
    {
        LOG(ERROR, "IRQ handler unregister failed");
        status = VAL_ERROR_POINT(12);
    }

rxtx_unmap:
    if (val_rxtx_unmap(sender))
    {
        LOG(ERROR, "RXTX_UNMAP failed");
        status = VAL_ERROR_POINT(13);
    }

free_memory:
   if (val_free(mb.recv) || val_free(mb.send))
    {
        LOG(ERROR, "free_rxtx_buffers failed");
        status = status ? status : VAL_ERROR_POINT(14);
    }

    if (val_free(pages))
    {
        LOG(ERROR, "val_free failed");
        status = status ? status : VAL_ERROR_POINT(15);
    }

    payload = val_select_server_fn_direct(test_run_data, 0, 0, 0, 0);

    return status ? status : (uint32_t)payload.arg3;
}

