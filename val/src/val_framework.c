/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_interfaces.h"
#include "val_common_peripherals.h"

/* Configurations that currently do not support NVM and WD */
#define SKIP_WD_PROGRAMMING
#define SKIP_NVM_PROGRAMMING

#ifdef SKIP_NVM_PROGRAMMING
#define NVM_SIZE (1024)
static uint8_t g_nvmem[NVM_SIZE];
#else
#define NVM_SIZE PLATFORM_NVM_SIZE
#endif

extern const uint32_t  total_tests;

/**
 * @brief  This API prints the testname and sets the test
 *         state to invalid.
 * @param  Test number
 * @return void
**/
void val_test_init(uint32_t test_num)
{
    uint32_t test_progress = TEST_START;
    const char *name_ptr = test_list[test_num].test_name;

    val_test_status_buffer_ts *status_buffer =
        (val_test_status_buffer_ts *)PLATFORM_SHARED_REGION_BASE;

    /* Advance one character to ignore empty space from test fixture macro*/
    name_ptr++;

    /* Clear test status */
    status_buffer->state        = TEST_FAIL;
    status_buffer->status_code  = VAL_STATUS_INVALID;

    LOG(ALWAYS, "Suite=%s : Test=%s\n", test_suite_list[test_list[test_num].suite_num].suite_desc,
        name_ptr);

    if (val_nvmem_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX), &test_progress, sizeof(uint32_t)))
    {
        VAL_PANIC("nvm write failed");
    }

    if (val_wd_enable())
    {
        VAL_PANIC("Watchdog enable failed");
    }
}

/**
 * @brief  This API prints the final test result
 * @param  none
 * @return test state
**/
void val_test_exit(void)
{
   uint32_t test_progress = TEST_END;

   if (val_wd_disable())
   {
      VAL_PANIC("Watchdog disable failed");
   }

   if (val_nvmem_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
           &test_progress, sizeof(uint32_t)))
   {
      VAL_PANIC("nvm write failed");
   }
}

/**
 * @brief  This API reloads the watchdog timer
 * @param  none
 * @return none
**/
void val_reprogram_watchdog(void)
{
   if (val_wd_disable())
   {
      VAL_PANIC("Watchdog disable failed");
   }
   if (val_wd_enable())
   {
      VAL_PANIC("Watchdog enable failed");
   }
}

/**
 *   @brief    - Check that an nvm access is within the bounds of the nvm
 *   @param    - offset  : Offset into nvm
 *               buffer  : Buffer address
 *               size    : Number of bytes
 *   @return   - SUCCESS/FAILURE
**/
static uint32_t nvm_check_bounds(uint32_t offset, void *buffer, size_t size)
{
    if (buffer == NULL)
        return VAL_ERROR;
    else if (offset > NVM_SIZE)
        return VAL_ERROR;
    else if (offset + size > NVM_SIZE)
        return VAL_ERROR;
    else if (size != sizeof(uint32_t))
        return VAL_ERROR;

    return VAL_SUCCESS;
}

/**
 *    @brief     - Writes 'size' bytes from buffer into non-volatile memory at a given
 *                 'base + offset'.
 *               - offset    : Offset
 *               - buffer    : Pointer to source address
 *               - size      : Number of bytes
 *    @return    - SUCCESS/FAILURE
**/
uint32_t val_nvmem_write(uint32_t offset, void *buffer, size_t size)
{
#ifndef SKIP_NVM_PROGRAMMING
   ffa_args_t  payload;
   uint32_t    data32 = *(uint32_t *)buffer;

   if (nvm_check_bounds(offset, buffer, size))
        return VAL_ERROR;

   if (val_get_curr_endpoint_logical_id() == SP1)
   {
      return val_nvm_write(offset, buffer, size);
   }
   else
   {
      val_memset(&payload, 0, sizeof(ffa_args_t));
      payload.arg1 = ((uint32_t)val_get_curr_endpoint_id() << 16) |
                                  val_get_endpoint_id(SP1);
      payload.arg3 = NVM_WRITE_SERVICE;
      payload.arg4 = offset;
      payload.arg5 = size;
      payload.arg6 = data32;
      val_ffa_msg_send_direct_req_32(&payload);
      if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
      {
         LOG(ERROR, "Invalid fid received, fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
         return VAL_ERROR;
      }

      return VAL_SUCCESS;
   }
#else
    if (nvm_check_bounds(offset, buffer, size))
    {
        return VAL_ERROR;
    }

    val_memcpy(g_nvmem + offset, buffer, size);
    return VAL_SUCCESS;
#endif
}

/**
 *   @brief - Reads 'size' bytes from Non-volatile memory 'base + offset' into given buffer.
 *              - offset    : Offset from NV MEM base address
 *              - buffer    : Pointer to destination address
 *              - size      : Number of bytes
 *   @return    - SUCCESS/FAILURE
**/
uint32_t val_nvmem_read(uint32_t offset, void *buffer, size_t size)
{
#ifndef SKIP_NVM_PROGRAMMING
   ffa_args_t  payload;
   if (nvm_check_bounds(offset, buffer, size))
        return VAL_ERROR;

   if (val_get_curr_endpoint_logical_id() == SP1)
   {
      return val_nvm_read(offset, buffer, size);
   }
   else
   {
      val_memset(&payload, 0, sizeof(ffa_args_t));
      payload.arg1 = ((uint32_t)val_get_curr_endpoint_id() << 16) |
                                  val_get_endpoint_id(SP1);
      payload.arg3 = NVM_READ_SERVICE;
      payload.arg4 = offset;
      payload.arg5 = size;
      val_ffa_msg_send_direct_req_32(&payload);
      if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
      {
         LOG(ERROR, "Invalid fid received, fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
         return VAL_ERROR;
      }

      *(uint32_t *)buffer = (uint32_t)payload.arg3;
      return VAL_SUCCESS;
   }
#else
    if (nvm_check_bounds(offset, buffer, size))
    {
        return VAL_ERROR;
    }

    val_memcpy(buffer, g_nvmem + offset, size);
    return VAL_SUCCESS;
#endif
}

/**
 *   @brief    - Initializes and enable the hardware watchdog timer
 *   @param    - void
 *   @return   - SUCCESS/FAILURE
**/
uint32_t val_wd_enable(void)
{
#ifndef SKIP_WD_PROGRAMMING
   ffa_args_t  payload;

   if (val_get_curr_endpoint_logical_id() == SP1)
   {
      return val_watchdog_enable();
   }
   else
   {
      val_memset(&payload, 0, sizeof(ffa_args_t));
      payload.arg1 = ((uint32_t)val_get_curr_endpoint_id() << 16) |
                                  val_get_endpoint_id(SP1);
      payload.arg3 = WD_ENABLE_SERVICE;
      val_ffa_msg_send_direct_req_32(&payload);
      if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
      {
         LOG(ERROR, "Invalid fid received, fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
         return VAL_ERROR;
      }

      return VAL_SUCCESS;
   }
#else
    return VAL_SUCCESS;
#endif
}

/**
 *   @brief    - Disables the hardware watchdog timer
 *   @param    - void
 *   @return   - SUCCESS/FAILURE
**/
uint32_t val_wd_disable(void)
{
#ifndef SKIP_WD_PROGRAMMING
   ffa_args_t  payload;
   uint64_t ep_info;

   if (val_get_curr_endpoint_logical_id() == SP1)
   {
      return val_watchdog_disable();
   }
   else
   {
      val_memset(&payload, 0, sizeof(ffa_args_t));
      payload.arg1 = ((uint32_t)val_get_curr_endpoint_id() << 16) |
                                  val_get_endpoint_id(SP1);
      payload.arg3 = WD_DISABLE_SERVICE;
      val_ffa_msg_send_direct_req_32(&payload);
      while (payload.fid == FFA_INTERRUPT_32)
      {
          ep_info = payload.arg1;
          val_memset(&payload, 0, sizeof(ffa_args_t));
          payload.arg1 = ep_info;
          val_ffa_run(&payload);
      }
      if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
      {
         LOG(ERROR, "Invalid fid received, fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
         return VAL_ERROR;
      }

      return VAL_SUCCESS;
   }
#else
    return VAL_SUCCESS;
#endif
}

uint32_t val_smmu_device_configure(uint32_t stream_id, uint64_t source, uint64_t dest,
                                     uint64_t size, bool secure)
{
    return pal_smmu_device_configure(stream_id, source, dest, size, secure);
}

/**
 *   @brief    - This function returns the test info of the last test that was run
 *   @param    - test_info address
 *   @return   - SUCCESS/FAILURE
**/
uint32_t val_get_last_run_test_info(test_info_t *test_info)
{
    uint32_t        reboot_run = 0;
    regre_report_t  regre_report = {0};
    uint8_t         test_progress_pattern[] = {TEST_START, TEST_END, TEST_FAIL, TEST_REBOOTING};

    if (val_nvmem_read(VAL_NVM_OFFSET(NVM_CUR_SUITE_NUM_INDEX),
            &test_info->suite_num, sizeof(uint32_t)))
        return VAL_ERROR;

    if (val_nvmem_read(VAL_NVM_OFFSET(NVM_CUR_TEST_NUM_INDEX),
            &test_info->test_num, sizeof(uint32_t)))
        return VAL_ERROR;

    if (val_nvmem_read(VAL_NVM_OFFSET(NVM_END_TEST_NUM_INDEX),
            &test_info->end_test_num, sizeof(uint32_t)))
        return VAL_ERROR;

    if (val_nvmem_read(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
            &test_info->test_progress, sizeof(uint32_t)))
        return VAL_ERROR;

    val_log_test_info(test_info);

    /* Is power on reset or warm reset? Determine based on NVM content */
    reboot_run = is_reboot_run(test_info->test_progress, test_progress_pattern,
                           sizeof(test_progress_pattern)/sizeof(test_progress_pattern[0]));

    /* Power on reset : Initiliase necessary data structure
     * Warm reset : Return previously executed test number
     * */
    if (!reboot_run)
    {
         val_reset_test_info_fields(test_info);
         val_reset_regression_report(&regre_report);

         if (val_nvmem_write(VAL_NVM_OFFSET(NVM_CUR_SUITE_NUM_INDEX),
                 &test_info->suite_num, sizeof(uint32_t)))
             return VAL_ERROR;
         if (val_nvmem_write(VAL_NVM_OFFSET(NVM_CUR_TEST_NUM_INDEX),
                 &test_info->test_num, sizeof(uint32_t)))
             return VAL_ERROR;

         if (val_nvmem_write(VAL_NVM_OFFSET(NVM_END_TEST_NUM_INDEX),
                 &test_info->end_test_num, sizeof(uint32_t)))
             return VAL_ERROR;

         if (val_nvmem_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
                 &test_info->test_progress, sizeof(uint32_t)))
             return VAL_ERROR;

         if (val_nvmem_write(VAL_NVM_OFFSET(NVM_TOTAL_PASS_INDEX),
                 &regre_report.total_pass, sizeof(uint32_t)))
             return VAL_ERROR;
         if (val_nvmem_write(VAL_NVM_OFFSET(NVM_TOTAL_FAIL_INDEX),
                 &regre_report.total_fail, sizeof(uint32_t)))
             return VAL_ERROR;
         if (val_nvmem_write(VAL_NVM_OFFSET(NVM_TOTAL_SKIP_INDEX),
                 &regre_report.total_skip, sizeof(uint32_t)))
             return VAL_ERROR;
         if (val_nvmem_write(VAL_NVM_OFFSET(NVM_TOTAL_ERROR_INDEX),
                 &regre_report.total_error, sizeof(uint32_t)))
             return VAL_ERROR;
    }

    val_log_final_test_status(test_info, &regre_report);
    return VAL_SUCCESS;
}

/**
 *   @brief    - This function notifies the framework about test
 *               intension of rebooting the platform. Test returns
 *               to framework on reset.
 *   @param    - Void
 *   @return   - Void
**/
void val_set_reboot_flag(void)
{
   uint32_t test_progress  = TEST_REBOOTING;

   LOG(INFO, "Setting reboot flag\n");
   if (val_nvmem_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
                 &test_progress, sizeof(uint32_t)))
   {
      VAL_PANIC("nvm write failed");
   }
}

/**
 *   @brief    - This function notifies the framework about test
 *               intension of not rebooting the platform.
 *   @param    - Void
 *   @return   - Void
**/
void val_reset_reboot_flag(void)
{
   uint32_t test_progress  = TEST_FAIL;

   LOG(INFO, "Resetting reboot flag\n");
   if (val_nvmem_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
                 &test_progress, sizeof(uint32_t)))
   {
      VAL_PANIC("nvm write failed");
   }
}

/**
 *   @brief    - This function is used valid EP configuration during test entry.
 *   @param    - EP ID
 *   @return   - Status
**/
uint32_t val_check_ep_compile_status(uint32_t client_logical_id, uint32_t server_logical_id)
{
    // Initialize EP Array Ptr
    val_endpoint_info_t *info_ptr = (val_endpoint_info_t *)val_get_endpoint_info();

    if (info_ptr[client_logical_id].is_valid == VAL_PARTITION_NOT_PRESENT ||
       ((info_ptr[server_logical_id].is_valid == VAL_PARTITION_NOT_PRESENT) &&
       (server_logical_id != NO_SERVER_EP)))
    {
        LOG(INFO, "EP Status invalid for client %x-->%x server %x-->%x, returning VAL_SKIP\n",
            client_logical_id, info_ptr[client_logical_id].is_valid, server_logical_id,
            info_ptr[server_logical_id].is_valid);
        return VAL_SKIP_CHECK;
    }
    return VAL_SUCCESS;
}


/**
 * @brief  This function sends a request with a Nil UUID to retrieve
 *         information about all partitions in the system. It compares
 *         the returned descriptors with the local endpoint info table
 *         and updates the compile_status accordingly.
 *
 * @param  None
 * @return None
 */
void val_ep_info_relayer_sync(void)
{
    val_endpoint_info_t *ep_info;
    ffa_partition_info_t *ret_info;
    void *rx_buff, *tx_buff;
    const uint32_t null_uuid[4] = {0};
    uint64_t size = PAGE_SIZE_4K;
    uint32_t i = 0, j = 0;
    uint32_t desc_count = 0;
    uint32_t ep_count = 5;//val_get_endpoint_info_table_count();
    ffa_args_t payload;

    tx_buff = val_aligned_alloc(PAGE_SIZE_4K, size);
    rx_buff = val_aligned_alloc(PAGE_SIZE_4K, size);
    if (rx_buff == NULL || tx_buff == NULL)
    {
        LOG(ERROR, "Failed to allocate RxTx buffer\n");
        goto free_memory;
    }

    /* Map TX and RX buffers for FFA communication */
    if (val_rxtx_map_64((uint64_t)tx_buff, (uint64_t)rx_buff, (uint32_t)(size / PAGE_SIZE_4K)))
    {
        LOG(ERROR, "RxTx Map failed\n");
        goto free_memory;
    }

    ep_info = val_get_endpoint_info();
    if (!ep_info)
    {
        LOG(ERROR, "Endpoint info retrieval failed\n");
        goto unmap_rxtx;
    }

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = null_uuid[0];
    payload.arg2 = null_uuid[1];
    payload.arg3 = null_uuid[2];
    payload.arg4 = null_uuid[3];
    payload.arg5 = FFA_PARTITION_INFO_FLAG_RETDESC;

    /* Request information for all partitions using the Nil UUID */
    val_ffa_partition_info_get(&payload);
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "ffa_partition_info_get failed\n");
        goto rx_release;
    }
    desc_count = (uint32_t)payload.arg2;
    ret_info = (ffa_partition_info_t *)rx_buff;

    /* Print all rx descriptors */
    LOG(DBG, "Partition Descriptor count: %d\n", (int)payload.arg2);
    for (i = 0; i < desc_count; i++) {
        ffa_partition_info_t *part = &ret_info[i];

        LOG(DBG, "+---------------- Partition Descriptor [%d] ----------------+\n", i);
        LOG(DBG, "| ID            : 0x%08x                               |\n", part->id);
        LOG(DBG, "| Exec Contexts : %-36u     |\n", part->exec_context);
        LOG(DBG, "| Properties    : 0x%08x                               |\n", part->properties);
        LOG(DBG, "| UUID          : %08x-%08x-%08x-%08x      |\n",
                 part->uuid[0], part->uuid[1], part->uuid[2], part->uuid[3]);
        LOG(DBG, "+----------------------------------------------------------+\n");
    }

    /* Validate returned descriptor size */
    if (payload.arg3 != sizeof(ffa_partition_info_t))
    {
        LOG(ERROR, "Expected desc size %zu, got %d\n",
            sizeof(ffa_partition_info_t), payload.arg2);
        goto rx_release;
    }

    /* Match each known endpoint against returned partition descriptors */
    for (i = 1; i < ep_count; i++)
    {
        int found = 0;

        for (j = 0; j < desc_count; j++)
        {
            if (ep_info[i].id == ret_info[j].id)
            {
                ep_info[i].is_valid = VAL_PARTITION_PRESENT;
                LOG(INFO, "Partition with EPID 0x%x Found\n", ep_info[i].id);
                found = 1;
                break;
            }
        }
        if (!found)
        {
            ep_info[i].is_valid = VAL_PARTITION_NOT_PRESENT;
            LOG(INFO, "Partition with EPID 0x%x Not Found\n", ep_info[i].id);
        }
    }

rx_release:
    /* Release the RX buffer */
    if (val_rx_release())
    {
        LOG(ERROR, "Rx release failed\n");
    }

unmap_rxtx:
    if (val_rxtx_unmap(val_get_endpoint_id(VM1)))
    {
        LOG(ERROR, "val_rxtx_unmap failed\n");
    }

free_memory:
    if (val_free(rx_buff) || val_free(tx_buff))
    {
        LOG(ERROR, "val_free failed\n");
    }
}


/**
 * @brief  Sends direct messages to SPs and Non Primary VM's to synchronize the endpoint info table.
 *
 *         This function iterates over all compiled Secure Partitions and synchronizes
 *         their endpoint info with Primary VM using two direct message calls (split payload).
 *
 * @param  None
 * @return None
 */
void val_send_sync_ep_info(void)
{
    ffa_args_t args[2] = {0};
    uint16_t count;
    uint32_t i, j;

    // Validate structure size matches serialized format size
    if (sizeof(val_endpoint_info_t) != VAL_ENDPOINT_INFO_SIZE)
    {
        LOG(ERROR, "val_endpoint_info_t size %d, required size %d\n",
            sizeof(val_endpoint_info_t), VAL_ENDPOINT_INFO_SIZE);
        VAL_PANIC("EP info struct size mismatch");
    }

    // Synchronize endpoint info for all SPs + VM1
    count = VAL_S_EP_COUNT + 1;

    // Get pointer to endpoint info table
    val_endpoint_info_t *info_ptr = (val_endpoint_info_t *)val_get_endpoint_info();
    for (i = 1; i <= VAL_S_EP_COUNT; i++)
    {
        // Skip partitions that are not present/compiled
        if (info_ptr[i].is_valid == VAL_PARTITION_NOT_PRESENT)
        {
            LOG(INFO, "Partition ID %x Not Compiled. Status: %x - Skipping Sync\n",
                info_ptr[i].id, info_ptr[i].is_valid);
            continue;
        }
        for (j = 1; j <= count; j++)
        {
            // Get pointer to the endpoint info structure to sync
            uint32_t *ptr = (uint32_t *)&info_ptr[j];

            // Clear argument structures
            val_memset(&args[0], 0, sizeof(ffa_args_t));
            val_memset(&args[1], 0, sizeof(ffa_args_t));

            // Set arg1: [sender << 16 | receiver] = [VM1 << 16 | info_ptr[i].id]
            args[0].arg1 = ((uint32_t)val_get_endpoint_id(VM1) << 16) | info_ptr[i].id;
            args[1].arg1 = args[0].arg1;

            // Set service ID and index (j) in arg3
            args[0].arg3 = (j << 16) | EP_INFO_SYNC_SERVICE;
            args[1].arg3 = args[0].arg3;

            // First half of ep_info structure
            args[0].arg4 = ptr[0];
            args[0].arg5 = ptr[1];
            args[0].arg6 = ptr[2];
            args[0].arg7 = ptr[3];

            // Second half of ep_info structure
            args[1].arg4 = ptr[4];
            args[1].arg5 = ptr[5];
            args[1].arg6 = ptr[6];
            args[1].arg7 = ptr[7];

            // Send both parts of the descriptor to the SP
            val_ffa_msg_send_direct_req_32(&args[0]);
            val_ffa_msg_send_direct_req_32(&args[1]);

            // Check for errors in direct message sends
            if (args[0].fid == FFA_ERROR_32 || args[1].fid == FFA_ERROR_32)
            {
                LOG(ERROR, "Direct Request 32 failed: err[0]=0x%x, err[1]=0x%x\n",
                    args[0].arg2, args[1].arg2);
                VAL_PANIC("EP Sync Error");
            }
        }
    }
}

/**
 * @brief  Service to Handle Endpoint Descriptor Sync with Primary VM.
 *
 *         This function is invoked when an Primary VM sends a direct message to update
 *         endpoint info table entry on the receiver's side. It unpacks the data sent
 *         in `ffa_args_t` and updates the corresponding `val_endpoint_info_t` entry.
 *
 * @param  args  Pointer to ffa_args_t received via direct message.
 * @return None
 */
void val_sync_ep_info_service(ffa_args_t *args)
{
    ffa_args_t payload;
    uint16_t count;
    uint32_t *ptr;
    val_endpoint_info_t ep_info;
    val_endpoint_info_t *ep_table_ptr;
    ffa_endpoint_id_t sender, receiver;
    uint32_t i = (uint32_t) args->arg3 >> 16;  // Extract table index from arg3 upper 16 bits

    // Check if the size of val_endpoint_info_t matches expected serialized size
    if (sizeof(val_endpoint_info_t) != VAL_ENDPOINT_INFO_SIZE)
    {
        VAL_PANIC("EP info struct size mismatch");
    }

    // Extract sender and receiver endpoint IDs from arg1
    sender = args->arg1 & 0x0000FFFF;
    receiver = (args->arg1 & 0xFFFF0000) >> 16;

    // Map structure to raw words for data transfer (8 uint32_t entries)
    ptr = (uint32_t *) &ep_info;
    ep_table_ptr = val_get_endpoint_info();
    (void)count; // 'count' currently unused

    // Deserialize the first half of val_endpoint_info_t from args
    ptr[0] = (uint32_t) args->arg4;
    ptr[1] = (uint32_t) args->arg5;
    ptr[2] = (uint32_t) args->arg6;
    ptr[3] = (uint32_t) args->arg7;

    // Prepare a direct response payload to acknowledge the received message
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)(sender << 16) | receiver);
    payload.arg2 = 0;

    val_ffa_msg_send_direct_resp_32(&payload);

    // Check for FFA error
    if (payload.fid == FFA_ERROR_32)
    {
        LOG(ERROR, "Direct Resp 32 failed err [0] %x [1] %x\n", payload.arg2, payload.arg3);
        VAL_PANIC("EP Sync Resp Error");
    }

    // Deserialize the second half of val_endpoint_info_t from response payload
    ptr[4] = (uint32_t) payload.arg4;
    ptr[5] = (uint32_t) payload.arg5;
    ptr[6] = (uint32_t) payload.arg6;
    ptr[7] = (uint32_t) payload.arg7;

    // Log and update endpoint info entry at index `i`
    LOG(DBG, "Updating val_endpoint_info[%x]:\n", i);
    LOG(DBG, "+------------+-----------------+-----------------+\n");
    LOG(DBG, "| Field      | New Value       | Old Value       |\n");
    LOG(DBG, "+------------+-----------------+-----------------+\n");
    LOG(DBG, "| name       | %-14s  | %-14s  |\n", ep_info.name, ep_table_ptr[i].name);
    val_memcpy(ep_table_ptr[i].name, ep_info.name, 5);
    LOG(DBG, "| id         | 0x%08x      | 0x%08x      |\n", ep_info.id, ep_table_ptr[i].id);
    ep_table_ptr[i].id = ep_info.id;
    LOG(DBG, "| valid      | 0x%08x      | 0x%08x      |\n",
        ep_info.is_valid, ep_table_ptr[i].is_valid);
    ep_table_ptr[i].is_valid = ep_info.is_valid;
    ep_table_ptr[i].id = ep_info.id;
    LOG(DBG, "| secure     | 0x%08x      | 0x%08x      |\n",
        ep_info.is_secure, ep_table_ptr[i].is_secure);
    ep_table_ptr[i].is_secure = ep_info.is_secure;
    LOG(DBG, "| tg0        | 0x%08x      | 0x%08x      |\n", ep_info.tg0, ep_table_ptr[i].tg0);
    ep_table_ptr[i].tg0 = ep_info.tg0;
    LOG(DBG, "| el_info    | 0x%08x      | 0x%08x      |\n",
        ep_info.el_info, ep_table_ptr[i].el_info);
    ep_table_ptr[i].el_info = ep_info.el_info;
    LOG(DBG, "| ec_count   | 0x%08x      | 0x%08x      |\n",
        ep_info.ec_count, ep_table_ptr[i].ec_count);
    ep_table_ptr[i].ec_count = ep_info.ec_count;
    LOG(DBG, "| properties | 0x%08x      | 0x%08x      |\n",
        ep_info.ep_properties, ep_table_ptr[i].ep_properties);
    ep_table_ptr[i].ep_properties = ep_info.ep_properties;
    for (int j = 0; j < 4; j++) {
        LOG(DBG, "| uuid[%d]    | 0x%08x      | 0x%08x      |\n",
            j, ep_info.uuid[j], ep_table_ptr[i].uuid[j]);
        ep_table_ptr[i].uuid[j] = ep_info.uuid[j];
    }
    LOG(DBG, "+------------+-----------------+-----------------+\n");
}