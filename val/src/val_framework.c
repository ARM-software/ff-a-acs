/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_framework.h"
#include "val_interfaces.h"

/* Global */
test_status_buffer_t    g_status_buffer = {
                            .state       = TEST_FAIL,
                            .status_code = VAL_STATUS_INVALID
                        };

/**
 *   @brief    - This function prints the given string and data onto the uart
 *   @param    - str      : Input String
 *             - data1    : Value for first format specifier
 *             - data2    : Value for second format specifier
 *   @return   - SUCCESS(0)/FAILURE
**/
uint32_t val_printf(const char *msg, uint64_t data1, uint64_t data2)
{
    char s[TOTAL_PRINT_LIMIT] = "";

    if (VERBOSITY <= DBG)
    {
        /* Prefix endpoint name to the string */
        val_strcat(s, (char *)val_get_curr_endpoint_name(), TOTAL_PRINT_LIMIT);
        val_strcat(s, ":  ", TOTAL_PRINT_LIMIT);
        val_strcat(s, (char *)msg, TOTAL_PRINT_LIMIT);
        return pal_printf(s, data1, data2);
    }
    else
    {
        return pal_printf(msg, data1, data2);
    }
}


/**
 *   @brief    - Parses input status for a given test and
 *               outputs appropriate information on the console
 *   @param    - Void
 *   @return   - Test state
**/
uint32_t val_report_status(uint32_t test_num)
{
    uint32_t status, status_code, state;
    char      test_result_print[PRINT_LIMIT] = "> TEST -";

    val_strcat(test_result_print,
                (char *)test_list[test_num].test_name,
                sizeof(test_result_print));

    status = val_get_status();
    state = (status >> TEST_STATE_SHIFT) & TEST_STATE_MASK;
    status_code = status & TEST_STATUS_CODE_MASK;

    switch (state)
    {
        case TEST_PASS:
        case TEST_PASS_WITH_SKIP:
            state = TEST_PASS;
            val_strcat(test_result_print, " => PASSED\n",
                sizeof(test_result_print));
            break;

        case TEST_SKIP:
            state = TEST_SKIP;
            val_strcat(test_result_print, " => SKIPPED (SKIP CODE=%d)\n",
                sizeof(test_result_print));
            break;

        case TEST_ERROR:
            state = TEST_ERROR;
            val_strcat(test_result_print, " => SIM ERROR (ERROR CODE=%d)\n",
                sizeof(test_result_print));
            break;
        default:
            state = TEST_FAIL;
            val_strcat(test_result_print, " => FAILED (ERROR CODE=%d)\n",
                sizeof(test_result_print));
            break;
    }

    LOG(ALWAYS, test_result_print, status_code, 0);
    LOG(ALWAYS, "\n", 0, 0);
    return state;
}

/**
 *   @brief    - Records the state and status of test
 *   @param    - Test status bit field - (state|status_code)
 *   @return   - void
**/
void val_set_status(uint32_t status)
{
    uint8_t state = ((status >> TEST_STATE_SHIFT) & TEST_STATE_MASK);

    g_status_buffer.status_code  = (status & TEST_STATUS_CODE_MASK);

    if ((g_status_buffer.state == TEST_PASS_WITH_SKIP && state == TEST_PASS) ||
        (g_status_buffer.state == TEST_PASS_WITH_SKIP && state == TEST_SKIP) ||
        (g_status_buffer.state == TEST_PASS && state == TEST_SKIP) ||
        (g_status_buffer.state == TEST_SKIP && state == TEST_PASS))
    {
        g_status_buffer.state = TEST_PASS_WITH_SKIP;
    }
    else
    {
        g_status_buffer.state = state;
    }
}

/**
 *   @brief    - Returns the state and status for a given test
 *   @param    - Void
 *   @return   - test status
**/
uint32_t val_get_status(void)
{
    return (uint32_t)(((g_status_buffer.state) << TEST_STATE_SHIFT) |
            (g_status_buffer.status_code));
}

/**
 * @brief  This API prints the testname and sets the test
 *         state to invalid.
 * @param  Test number
 * @return void
**/

void val_test_init(uint32_t test_num)
{
   char testname[PRINT_LIMIT] = "> TEST -";
   uint32_t test_progress = TEST_START;

   /* Clear test status */
   val_set_status(RESULT_FAIL(VAL_STATUS_INVALID));

   val_strcat(testname,
                (char *)test_list[test_num].test_name,
                sizeof(testname));
   val_strcat(testname, " => START\n", sizeof(testname));

   LOG(ALWAYS, "\n", 0, 0);
   LOG(ALWAYS, testname, 0, 0);

   if (val_nvm_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
           &test_progress, sizeof(uint32_t)))
   {
      VAL_PANIC("\tnvm write failed\n");
   }

   if (val_watchdog_enable())
   {
      VAL_PANIC("\tWatchdog enable failed\n");
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

   if (val_watchdog_disable())
   {
      VAL_PANIC("\tWatchdog disable failed\n");
   }

   if (val_nvm_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
           &test_progress, sizeof(uint32_t)))
   {
      VAL_PANIC("\tnvm write failed\n");
   }
}

/**
 * @brief  This API reloads the watchdog timer
 * @param  none
 * @return none
**/
void val_reprogram_watchdog(void)
{
   if (val_watchdog_disable())
   {
      VAL_PANIC("\tWatchdog disable failed\n");
   }
   if (val_watchdog_enable())
   {
      VAL_PANIC("\tWatchdog enable failed\n");
   }
}
/**
 *   @brief    - Check that an nvm access is within the bounds of the nvm
 *   @param    - base    : Base address of nvm (must be zero)
 *               offset  : Offset into nvm
 *               size    : Number of bytes
 *   @return   - SUCCESS/FAILURE
**/
static uint32_t nvm_check_bounds(uint32_t offset, void *buffer, size_t size)
{
    if (buffer == NULL)
        return VAL_ERROR;
    else if (offset > PLATFORM_NVM_SIZE)
        return VAL_ERROR;
    else if (offset + size > PLATFORM_NVM_SIZE)
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
uint32_t val_nvm_write(uint32_t offset, void *buffer, size_t size)
{
   ffa_args_t  payload;
   uint32_t    data32 = *(uint32_t*)buffer;

   if (nvm_check_bounds(offset, buffer, size))
        return VAL_ERROR;

   if (val_get_curr_endpoint_logical_id() == SP1)
   {
      return pal_nvm_write(offset, buffer, size);
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
         LOG(ERROR, "\tInvalid fid received, fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
         return VAL_ERROR;
      }

      return VAL_SUCCESS;
   }
}

/**
 *   @brief - Reads 'size' bytes from Non-volatile memory 'base + offset' into given buffer.
 *              - offset    : Offset from NV MEM base address
 *              - buffer    : Pointer to destination address
 *              - size      : Number of bytes
 *   @return    - SUCCESS/FAILURE
**/
uint32_t val_nvm_read(uint32_t offset, void *buffer, size_t size)
{
   ffa_args_t  payload;
   if (nvm_check_bounds(offset, buffer, size))
        return VAL_ERROR;

   if (val_get_curr_endpoint_logical_id() == SP1)
   {
      return pal_nvm_read(offset, buffer, size);
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
         LOG(ERROR, "\tInvalid fid received, fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
         return VAL_ERROR;
      }

      *(uint32_t*)buffer = (uint32_t)payload.arg3;
      return VAL_SUCCESS;
   }
}

/**
 *   @brief    - Initializes and enable the hardware watchdog timer
 *   @param    - void
 *   @return   - SUCCESS/FAILURE
**/
uint32_t val_watchdog_enable(void)
{
   ffa_args_t  payload;

   if (val_get_curr_endpoint_logical_id() == SP1)
   {
      return pal_watchdog_enable();
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
         LOG(ERROR, "\tInvalid fid received, fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
         return VAL_ERROR;
      }

      return VAL_SUCCESS;
   }
}

/**
 *   @brief    - Disables the hardware watchdog timer
 *   @param    - void
 *   @return   - SUCCESS/FAILURE
**/
uint32_t val_watchdog_disable(void)
{
   ffa_args_t  payload;

   if (val_get_curr_endpoint_logical_id() == SP1)
   {
      return pal_watchdog_disable();
   }
   else
   {
      val_memset(&payload, 0, sizeof(ffa_args_t));
      payload.arg1 = ((uint32_t)val_get_curr_endpoint_id() << 16) |
                                  val_get_endpoint_id(SP1);
      payload.arg3 = WD_DISABLE_SERVICE;
      val_ffa_msg_send_direct_req_32(&payload);
      if (payload.fid != FFA_MSG_SEND_DIRECT_RESP_32)
      {
         LOG(ERROR, "\tInvalid fid received, fid=0x%x, err=0x%x\n", payload.fid, payload.arg2);
         return VAL_ERROR;
      }

      return VAL_SUCCESS;
   }
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
    uint32_t        reboot_run = 0, i = 0;
    regre_report_t  regre_report = {0};
    uint8_t         test_progress_pattern[] = {TEST_START, TEST_END, TEST_FAIL, TEST_REBOOTING};

    if (val_nvm_read(VAL_NVM_OFFSET(NVM_CUR_SUITE_NUM_INDEX),
            &test_info->suite_num, sizeof(uint32_t)))
        return VAL_ERROR;

    if (val_nvm_read(VAL_NVM_OFFSET(NVM_CUR_TEST_NUM_INDEX),
            &test_info->test_num, sizeof(uint32_t)))
        return VAL_ERROR;

    if (val_nvm_read(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
            &test_info->test_progress, sizeof(uint32_t)))
        return VAL_ERROR;

    LOG(INFO, "\tIn val_get_last_run_test_info, test_num=%x\n", test_info->test_num, 0);
    LOG(INFO, "\tsuite_num=%x\n", test_info->suite_num, 0);
    LOG(INFO, "\ttest_progress=%x\n", test_info->test_progress, 0);

    /* Is power on reset or warm reset? Determine based on NVM content */
    while (i < (uint32_t)(sizeof(test_progress_pattern)/sizeof(test_progress_pattern[0])))
    {
        if (test_info->test_progress == test_progress_pattern[i])
        {
            reboot_run = 1;
            break;
        }
        i++;
    }

    /* Power on reset : Initiliase necessary data structure
     * Warm reset : Return previously executed test number
     * */
    if (!reboot_run)
    {
         test_info->test_num         = VAL_INVALID_TEST_NUM;
         test_info->suite_num         = 0;
         test_info->test_progress     = 0;
         val_memset(&regre_report, 0x0, sizeof(regre_report_t));

         if (val_nvm_write(VAL_NVM_OFFSET(NVM_CUR_SUITE_NUM_INDEX),
                 &test_info->suite_num, sizeof(uint32_t)))
             return VAL_ERROR;
         if (val_nvm_write(VAL_NVM_OFFSET(NVM_CUR_TEST_NUM_INDEX),
                 &test_info->test_num, sizeof(uint32_t)))
             return VAL_ERROR;
         if (val_nvm_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
                 &test_info->test_progress, sizeof(uint32_t)))
             return VAL_ERROR;

         if (val_nvm_write(VAL_NVM_OFFSET(NVM_TOTAL_PASS_INDEX),
                 &regre_report.total_pass, sizeof(uint32_t)))
             return VAL_ERROR;
         if (val_nvm_write(VAL_NVM_OFFSET(NVM_TOTAL_FAIL_INDEX),
                 &regre_report.total_fail, sizeof(uint32_t)))
             return VAL_ERROR;
         if (val_nvm_write(VAL_NVM_OFFSET(NVM_TOTAL_SKIP_INDEX),
                 &regre_report.total_skip, sizeof(uint32_t)))
             return VAL_ERROR;
         if (val_nvm_write(VAL_NVM_OFFSET(NVM_TOTAL_ERROR_INDEX),
                 &regre_report.total_error, sizeof(uint32_t)))
             return VAL_ERROR;
    }

    LOG(INFO, "\tIn val_get_last_run_test_num, test_num=%x\n", test_info->test_num, 0);
    LOG(INFO, "\tsuite_num=%x\n", test_info->suite_num, 0);
    LOG(INFO, "\tregre_report.total_pass=%x\n", regre_report.total_pass, 0);
    LOG(INFO, "\tregre_report.total_fail=%x\n", regre_report.total_fail, 0);
    LOG(INFO, "\tregre_report.total_skip=%x\n", regre_report.total_skip, 0);
    LOG(INFO, "\tregre_report.total_error=%x\n", regre_report.total_error, 0);
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

   LOG(INFO, "\tSetting reboot flag\n", 0, 0);
   if (val_nvm_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
                 &test_progress, sizeof(uint32_t)))
   {
      VAL_PANIC("\tnvm write failed\n");
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

   LOG(INFO, "\tResetting reboot flag\n", 0, 0);
   if (val_nvm_write(VAL_NVM_OFFSET(NVM_TEST_PROGRESS_INDEX),
                 &test_progress, sizeof(uint32_t)))
   {
      VAL_PANIC("\tnvm write failed\n");
   }
}
