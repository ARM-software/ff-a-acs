/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_interfaces.h"
#include "val_common_peripherals.h"

#if (PLATFORM_SP_EL == -1)
#define SKIP_WD_PROGRAMMING
#define SKIP_NVM_PROGRAMMING
#endif

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
