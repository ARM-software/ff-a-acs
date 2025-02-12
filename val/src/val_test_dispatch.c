/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_test_dispatch.h"

#ifndef TARGET_LINUX
extern uint64_t val_image_load_offset;
#else
uint64_t val_image_load_offset = 0;
#endif
mp_test_status_t g_mp_state = {
    .g_other_pe_test_state = {[0 ... PLATFORM_NO_OF_CPUS - 1] = VAL_MP_STATE_WAIT},
    .g_other_pe_test_result = {[0 ... (PLATFORM_NO_OF_CPUS - 1)] = VAL_STATUS_INVALID}
};

extern const uint32_t  total_tests;

/* Validate endpoint config for the given system */
static uint32_t validate_test_config(uint32_t client_logical_id __UNUSED,
                                     uint32_t server_logical_id __UNUSED)
{
#if (PLATFORM_NS_HYPERVISOR_PRESENT == 0)
    if (client_logical_id == VM2 || client_logical_id == VM3
        || server_logical_id == VM2 || server_logical_id == VM3)
    {
        LOG(TEST, "No support for ns-hyp, skipping the check for client %s server %s ",
            val_get_endpoint_name(client_logical_id), val_get_endpoint_name(server_logical_id));
        return VAL_SKIP_CHECK;
    }
#endif

// PLATFORM doesn't support deploying FFA based SPs
#if (PLATFORM_SP_EL == -1)
    if (client_logical_id == SP1 || server_logical_id == SP1
        || client_logical_id == SP2 || server_logical_id == SP2
        || client_logical_id == SP3 || server_logical_id == SP3
        || client_logical_id == SP4 || server_logical_id == SP4)
    {
        LOG(TEST, "No support for FFA S-ENDPOINT, skipping the check for client %s server %s ",
            val_get_endpoint_name(client_logical_id), val_get_endpoint_name(server_logical_id));
        return VAL_SKIP_CHECK;
    }
#endif

#if (PLATFORM_SPMC_EL == 1 && PLATFORM_SP_EL == 1)
    if (client_logical_id == SP2 || client_logical_id == SP3
        || server_logical_id == SP2 || server_logical_id == SP3
        || client_logical_id == SP4 || server_logical_id == SP4)
    {
        LOG(TEST, "Both SP & SPMC at EL1 config isn't supported, skipping the check");
        return VAL_SKIP_CHECK;
    }
#endif

#if (PLATFORM_SP_SEND_DIRECT_REQ == 0)
    if (client_logical_id <= SP4 && server_logical_id != NO_SERVER_EP)
    {
        LOG(TEST, "SP doesn't support DIRECT_REQ, skipping the check");
        return VAL_SKIP_CHECK;
    }
#endif

#if (PLATFORM_VM_SEND_DIRECT_RESP == 0)
    if (server_logical_id > SP4 && server_logical_id != NO_SERVER_EP)
    {
        LOG(TEST, "NS-ENDPOINT doesn't support DIRECT_RESP,skipping the check");
        return VAL_SKIP_CHECK;
    }
#endif
    return VAL_SUCCESS;
}

/**
 *   @brief    - Execute client test fn of given test
 *   @param    - test runtime data such as test_num, client-server ids
 *               participating in the test scenario
 *   @return   - status
**/
static uint32_t val_execute_client_test_fn(uint32_t test_run_data)
{
    uint32_t       status = VAL_SUCCESS;
    uint32_t       test_num = GET_TEST_NUM(test_run_data);
    client_test_t client_fn_ptr;

    client_fn_ptr = (client_test_t)(test_list[test_num].client_fn);
    if (client_fn_ptr == NULL)
    {
       VAL_PANIC("Invalid client test function");
    }
    /* Execute client function of given test num */
    client_fn_ptr = client_fn_ptr + val_image_load_offset;
    status = client_fn_ptr(test_run_data);

    return status;
}

/**
 *   @brief    - Executes given server test function of the test
 *   @param    - Direct message request parameters sent by client endpoint
 *   @return   - status
**/
static uint32_t val_execute_server_test_fn(ffa_args_t args)
{
    uint32_t       test_num = GET_TEST_NUM(args.arg3);
    uint32_t       status = VAL_ERROR;
    server_test_t  server_fn_ptr;

    server_fn_ptr = (server_test_t)(test_list[test_num].server_fn);
    if (server_fn_ptr == NULL)
    {
       VAL_PANIC("Invalid server test function");
    }
    /* Execute server function of given test num */
    server_fn_ptr = server_fn_ptr + val_image_load_offset;
    status = server_fn_ptr(args);

    return status;
}

/**
 *   @brief    - Execute test suite
 *   @param    - void
 *   @return   - void
**/
void val_run_test_suite(void)
{
    uint32_t my_logical_id = val_get_curr_endpoint_logical_id();
    uint8_t  el_info = val_get_curr_endpoint_el_info();

    /* Dispatcher VM(VM1) is the master of the test suite and
     * drives the test regression by calling test_entry
     * function of each test one by one.
     *
     * Other endpoints wait for request and execute client_tests
     * or server_test_fn based on the command received from the
     * dispatcher vm.
     */
    if (my_logical_id == VM1)
    {
        val_test_dispatch();
    }
    else
    {
        if ((my_logical_id == SP1 || my_logical_id == SP2)
             && GET_EL_NUM(el_info) == EL1)
        {
            val_ffa_secondary_ep_register_64();
        }
        val_wait_for_test_fn_req();
        VAL_PANIC("Something wrong, shouldn't have reached here");
    }
}

/**
 *   @brief    - Print ACS header
 *   @param    - void
 *   @return   - void
**/
static void val_print_acs_header(void)
{
   LOG(ALWAYS, "===========================================================");
   LOG(ALWAYS, "\t\t***** FF-A v%d.%d ACS EAC *****",
            FFA_VERSION_MAJOR, FFA_VERSION_MINOR);
   LOG(ALWAYS, "===========================================================");
}

/**
 *   @brief    - Query test database and execute test from each suite one by one
 *   @param    - void
 *   @return   - void
**/
void val_test_dispatch(void)
{
    regre_report_t    regre_report = {0};
    test_info_t       test_info = {0};
    uint32_t          test_result, test_num, suite_num, i, reboot_run = 0;
    uint32_t          test_num_start = 0, test_num_end = 0;
    test_entry_fptr_t test_entry_fn_ptr;

    if (val_get_last_run_test_info(&test_info))
    {
        LOG(ERROR, "Unable to read last test_info");
        return;
    }

    if (test_info.test_num == VAL_INVALID_TEST_NUM)
    {
        val_print_acs_header();
        test_info.test_num = 1;
    }
    else
    {
        reboot_run = 1;
    }

    suite_num = test_info.suite_num;

    if (!reboot_run)
    {
#if defined(SUITE_TEST_RANGE)
        uint32_t test_num, j;
        char *start_test_name = SUITE_TEST_RANGE_MIN;
        char *end_test_name = SUITE_TEST_RANGE_MAX;
        char *data_base = NULL;

        test_num = test_info.test_num;
        for (; test_num < total_tests-1; test_num++)
        {
            data_base = (char *)test_list[test_num].test_name;
            data_base++;
            if (val_strcmp((char *)data_base, start_test_name) == 0)
            {
                test_num_start = test_num;
            }
        }

        test_num = test_info.test_num;
        for (; test_num < total_tests-1; test_num++)
        {
            data_base = (char *)test_list[test_num].test_name;
            data_base++;
            if (val_strcmp((char *)data_base, end_test_name) == 0)
            {
                test_num_end = test_num;
            }
        }

        if (test_num_start > test_num_end)
        {
            j = test_num_start;
            test_num_start = test_num_end;
            test_num_end = j;
        }

        if ((val_nvm_write(VAL_NVM_OFFSET(NVM_END_TEST_NUM_INDEX),
                                                  &test_num_end, sizeof(test_num_end))))
        {
            LOG(ERROR, "Unable to write nvm");
            return;
        }
#else
        test_num_start = test_info.test_num;
        test_num_end = total_tests;
        if ((val_nvm_write(VAL_NVM_OFFSET(NVM_END_TEST_NUM_INDEX),
                                                  &test_num_end, sizeof(test_num_end))))
        {
            LOG(ERROR, "Unable to write nvm");
            return;
        }
#endif
    }
    else
    {
        test_num_start = test_info.test_num;
        test_num_end = test_info.end_test_num;
    }

    /* Iterate over test_list[] to run test one by one */
    for (i = test_num_start ; i <= test_num_end; i++)
    {
        if (test_list[i].testentry_fn == NULL)
            break;

        /* Fix symbol relocation - Add image offset */
        test_entry_fn_ptr = (test_entry_fptr_t)(test_list[i].testentry_fn
                                + val_image_load_offset);
        if (reboot_run)
        {
            /* Reboot case, find out whether reboot expected or not? */
            if (test_info.test_progress == TEST_REBOOTING)
            {
                /* Reboot expected, declare previous test as pass */
                val_set_status(RESULT_PASS(VAL_SUCCESS));
            }
            else
            {
                /* Reboot not expected, declare previous test as error */
                val_set_status(RESULT_ERROR(VAL_SIM_ERROR));
            }
            reboot_run = 0;
        }
        else
        {

            if (suite_num != test_list[i].suite_num)
            {
                /* Print test suite name */
                suite_num = test_list[i].suite_num;
            }

            if ((val_nvm_write(VAL_NVM_OFFSET(NVM_CUR_SUITE_NUM_INDEX),
                    &suite_num, sizeof(suite_num))) ||
                (val_nvm_write(VAL_NVM_OFFSET(NVM_CUR_TEST_NUM_INDEX),
                    &test_num, sizeof(test_num))))
            {
                LOG(ERROR, "Unable to write nvm");
                return;
            }


           /* g_current_test_num is used for secondary cpu power on
            * therefore perform cache ops on location to avoid any
            * sharing issue
            * */
           g_mp_state.g_current_test_num = i;
           val_dataCacheCleanInvalidateVA((uint64_t)&g_mp_state);

           val_test_init(i);

           /* Execute test, Call <testname>testentry() function */
           test_entry_fn_ptr(i);

           val_test_exit();
        }

        test_result = val_report_status(i);

        if (val_nvm_read(VAL_NVM_OFFSET(NVM_TOTAL_PASS_INDEX),
                 &regre_report.total_pass, sizeof(uint32_t)) ||
            val_nvm_read(VAL_NVM_OFFSET(NVM_TOTAL_FAIL_INDEX),
                 &regre_report.total_fail, sizeof(uint32_t))  ||
            val_nvm_read(VAL_NVM_OFFSET(NVM_TOTAL_SKIP_INDEX),
                 &regre_report.total_skip, sizeof(uint32_t))  ||
            val_nvm_read(VAL_NVM_OFFSET(NVM_TOTAL_ERROR_INDEX),
                 &regre_report.total_error, sizeof(uint32_t)))

        {
            LOG(ERROR, "Unable to read regre_report");
            return;
        }

        switch (test_result)
        {
            case TEST_PASS:
                regre_report.total_pass++;
                break;
            case TEST_FAIL:
                regre_report.total_fail++;
                break;
            case TEST_SKIP:
                regre_report.total_skip++;
                break;
            case TEST_ERROR:
                regre_report.total_error++;
                break;
        }

        if (val_nvm_write(VAL_NVM_OFFSET(NVM_TOTAL_PASS_INDEX),
                 &regre_report.total_pass, sizeof(uint32_t)) ||
            val_nvm_write(VAL_NVM_OFFSET(NVM_TOTAL_FAIL_INDEX),
                 &regre_report.total_fail, sizeof(uint32_t))  ||
            val_nvm_write(VAL_NVM_OFFSET(NVM_TOTAL_SKIP_INDEX),
                 &regre_report.total_skip, sizeof(uint32_t))  ||
            val_nvm_write(VAL_NVM_OFFSET(NVM_TOTAL_ERROR_INDEX),
                 &regre_report.total_error, sizeof(uint32_t)))
        {
            LOG(ERROR, "Unable to write regre_report");
            return;
        }
    }

    /* Print Regression report */
    LOG(ALWAYS, "");
    LOG(ALWAYS, "REGRESSION REPORT: ");
    LOG(ALWAYS, "==========================");
    LOG(ALWAYS, "   TOTAL TESTS     : %d",
        (uint64_t)(regre_report.total_pass
        + regre_report.total_fail
        + regre_report.total_skip
        + regre_report.total_error),
        0);
    LOG(ALWAYS, "   TOTAL PASSED    : %d", regre_report.total_pass);
    LOG(ALWAYS, "   TOTAL FAILED    : %d", regre_report.total_fail);
    LOG(ALWAYS, "   TOTAL SKIPPED   : %d", regre_report.total_skip);
    LOG(ALWAYS, "   TOTAL SIM ERROR : %d", regre_report.total_error);
    LOG(ALWAYS, "==========================");
    LOG(ALWAYS, "******* END OF ACS *******");
}

/**
 *   @brief    - Test entry for non-dispatcher endpoints. This executes
 *               client or server test functions based on dispatcher's command
 *   @param    - void
 *   @return   - void
**/
void val_wait_for_test_fn_req(void)
{
    ffa_args_t        payload;
    uint32_t          test_run_data;
    uint32_t          status = VAL_ERROR;
    ffa_endpoint_id_t target_id, my_id;
    uint32_t          buffer;

    val_memset(&payload, 0, sizeof(ffa_args_t));

    LOG(ALWAYS, "Enter Wait State, Partition ID %x", val_get_curr_endpoint_logical_id());
    /* Receive the test_num and client_fn/server_fn to run
     * OR reciever service id for nvm and wd functionality
     */
    val_ffa_msg_wait(&payload);

    while (1)
    {
        if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
        {
            LOG(ERROR, "Invalid fid received, fid=0x%x, error=0x%x", payload.fid, payload.arg2);
            return;
        }

        target_id = SENDER_ID(payload.arg1);
        my_id = RECEIVER_ID(payload.arg1);
        test_run_data = (uint32_t)payload.arg3;

        switch (test_run_data)
        {
            case NVM_WRITE_SERVICE:
            buffer = (uint32_t) payload.arg6;
            if (val_nvm_write((uint32_t)payload.arg4, &buffer, payload.arg5))
            {
               VAL_PANIC("nvm write failed");
            }
            val_memset(&payload, 0, sizeof(ffa_args_t));
            payload.arg1 = ((uint32_t)my_id << 16) | target_id;
            val_ffa_msg_send_direct_resp_32(&payload);
            break;

            case NVM_READ_SERVICE:
            if (val_nvm_read((uint32_t)payload.arg4, &buffer, payload.arg5))
            {
               VAL_PANIC("nvm write failed");
            }
            val_memset(&payload, 0, sizeof(ffa_args_t));
            payload.arg1 = ((uint32_t)my_id << 16) | target_id;
            payload.arg3 = buffer;
            val_ffa_msg_send_direct_resp_32(&payload);
            break;

            case WD_ENABLE_SERVICE:
            if (val_watchdog_enable())
            {
               VAL_PANIC("Watchdog enable failed");
            }
            val_memset(&payload, 0, sizeof(ffa_args_t));
            payload.arg1 = ((uint32_t)my_id << 16) | target_id;
            val_ffa_msg_send_direct_resp_32(&payload);
            break;

            case WD_DISABLE_SERVICE:
            if (val_watchdog_disable())
            {
               VAL_PANIC("Watchdog disable failed");
            }
            val_memset(&payload, 0, sizeof(ffa_args_t));
            payload.arg1 = ((uint32_t)my_id << 16) | target_id;
            val_ffa_msg_send_direct_resp_32(&payload);
            break;

            /* Handle test function execution */
            default:
            if (GET_TEST_TYPE(test_run_data) == SERVER_TEST)
            {
                status = val_execute_server_test_fn(payload);
            }
            else
            {
                /* Execute client tests for given client endpoint */
                status = val_execute_client_test_fn(test_run_data);
            }

            /* Send test status back */
            val_memset(&payload, 0, sizeof(ffa_args_t));
            payload.arg1 = ((uint32_t)my_id << 16) | target_id;
            payload.arg3 = status;
            val_ffa_msg_send_direct_resp_32(&payload);
        }
    }
}

/**
 *   @brief    - Execute test functions for given client-server endpoint ids
 *   @param    - test runtime data such as test_num, client-server ids
 *               participating in the test scenario
 *   @return   - status
**/
uint32_t val_execute_test(
                uint32_t test_num,
                uint32_t client_logical_id,
                uint32_t server_logical_id)
{
    uint32_t        status = VAL_ERROR;
    ffa_args_t      payload;
    uint32_t        test_run_data;

    test_run_data = TEST_RUN_DATA(test_num,
                                  client_logical_id,
                                  server_logical_id,
                                  CLIENT_TEST);

    if (server_logical_id != NO_SERVER_EP)
    {
        LOG(TEST, "Executing Test Setup client: %s server: %s",
                val_get_endpoint_name(client_logical_id), val_get_endpoint_name(server_logical_id));
    }
    else
    {
        LOG(TEST, "Executing Test Setup client: %s", val_get_endpoint_name(client_logical_id));
    }

    status = validate_test_config(client_logical_id, server_logical_id);
    if (status)
    {
        goto exit;
    }

    if (client_logical_id == VM1)
    {
        status = val_execute_client_test_fn(test_run_data);
        goto exit;
    }
    else if (server_logical_id == VM1)
    {
        LOG(ERROR, "Unsupported: VM1 can't be server_ep");
        status = VAL_ERROR;
        goto exit;
    }
    else
    {
        /* Release the given client endpoint from wait
         * (in val_wait_for_test_fn_req) so that it can
         * execute client tests.
         */
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = ((uint32_t)val_get_curr_endpoint_id() << 16) |
                                    val_get_endpoint_id(client_logical_id);
        payload.arg3 = test_run_data;
        val_ffa_msg_send_direct_req_32(&payload);

        /* Check the test status */
        if (payload.fid == FFA_MSG_SEND_DIRECT_RESP_32)
        {
            status = (uint32_t)payload.arg3;
            goto exit;
        }
        else
        {
            LOG(ERROR, "Invalid fid received, fid=0x%x arg1=0x%x arg2=0x%x", \
                payload.fid,  payload.arg1, payload.arg2);
            status = VAL_ERROR;
            goto exit;
        }
    }

exit:
    if (status == VAL_SKIP_CHECK)
    {
        val_set_status(RESULT_SKIP(status));
        return  RESULT_SKIP(status);
    }
    else if (status == VAL_SUCCESS)
    {
        val_set_status(RESULT_PASS(status));
        return  RESULT_PASS(status);
    }
    else
    {
        val_set_status(RESULT_FAIL(status));
        return  RESULT_FAIL(status);
    }
}

/**
 *   @brief    - Handshake with given server endpoint to execute server test fn
 *   @param    - test runtime data such as test_num, client-server ids
 *               participating in the test scenario
 *   @param    - arg4 to arg7 - test specific data
 *   @return   - FFA_MSG_SEND_DIRECT_RESP_32 return args
**/
ffa_args_t val_select_server_fn_direct(uint32_t test_run_data,
                                       uint32_t arg4,
                                       uint32_t arg5,
                                       uint32_t arg6,
                                       uint32_t arg7)
{
    ffa_args_t      payload;
    uint32_t        client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t        server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);

    /* Add server_test type */
    test_run_data = ADD_TEST_TYPE(test_run_data, SERVER_TEST);

    /* Release the given server endpoint from wait
     * (in val_wait_for_test_fn_req) so that it can
     * execute given server test fn.
     */
    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(client_logical_id) << 16) |
                                val_get_endpoint_id(server_logical_id);
    payload.arg3 = test_run_data;
    payload.arg4 = arg4;
    payload.arg5 = arg5;
    payload.arg6 = arg6;
    payload.arg7 = arg7;
    val_ffa_msg_send_direct_req_32(&payload);

    return payload;
}

/**
 *   @brief    - Unblock the client and wait for new message request
 *   @param    - test runtime data such as test_num, client-server ids
 *               participating in the test scenario
 *   @param    - arg4 to arg7 - test specific data
 *   @return   - ffa_msg_send_direct_req_32 return values
**/
ffa_args_t val_resp_client_fn_direct(uint32_t test_run_data,
                                       uint32_t arg3,
                                       uint32_t arg4,
                                       uint32_t arg5,
                                       uint32_t arg6,
                                       uint32_t arg7)
{
    ffa_args_t      payload;
    uint32_t        client_logical_id = GET_CLIENT_LOGIC_ID(test_run_data);
    uint32_t        server_logical_id = GET_SERVER_LOGIC_ID(test_run_data);

    val_memset(&payload, 0, sizeof(ffa_args_t));
    payload.arg1 = ((uint32_t)val_get_endpoint_id(server_logical_id) << 16) |
                                val_get_endpoint_id(client_logical_id);
    payload.arg3 = arg3;
    payload.arg4 = arg4;
    payload.arg5 = arg5;
    payload.arg6 = arg6;
    payload.arg7 = arg7;

    val_ffa_msg_send_direct_resp_32(&payload);
    return payload;
}

/**
 *   @brief    - VAL API to execute test on secondary cpu
 *   @param    - Void
 *   @return   - Void
**/
void val_secondary_cpu_test_entry(void)
{
    sec_cpu_client_test_t      sec_cpu_client_fn_ptr;
    uint32_t status = VAL_ERROR;
    uint64_t mpid = val_read_mpidr() & MPID_MASK;
    uint32_t cpu_id = val_get_cpuid(mpid);;

    if (val_get_curr_endpoint_logical_id() == VM1)
    {
        /* Set MP Test state context to in progress */
        g_mp_state.g_other_pe_test_state[cpu_id] = VAL_MP_STATE_INPROGRESS;
        val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_other_pe_test_state);
        val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_current_test_num);

        sec_cpu_client_fn_ptr = \
          (sec_cpu_client_test_t)(test_list[g_mp_state.g_current_test_num].sec_cpu_client_fn);
        if (sec_cpu_client_fn_ptr == NULL)
        {
            VAL_PANIC("Invalid sec cpu test function");
        }

        sec_cpu_client_fn_ptr = sec_cpu_client_fn_ptr + val_image_load_offset;
        /* Execute sec cpu test function */
        status = sec_cpu_client_fn_ptr(g_mp_state.g_current_test_num);

        /* Update global Multi PE test state */
        g_mp_state.g_other_pe_test_result[cpu_id] = status;
        g_mp_state.g_other_pe_test_state[cpu_id] = VAL_MP_STATE_COMPLETE;
        val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_other_pe_test_result);
        val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_other_pe_test_state);

        /* Power off Secondary PE after test run*/
        val_power_off_cpu();
    }
    else
    {
        /* unsused */
        (void)status;
        (void)cpu_id;
        (void)mpid;

        val_sec_cpu_wait_for_test_fn_req();
    }
}

/**
 *   @brief    - Executes given server test function of the test on Sec CPU
 *   @param    - Direct message request parameters sent by client endpoint
 *   @return   - status
**/
static uint32_t val_sec_cpu_execute_server_test_fn(ffa_args_t args)
{
    uint32_t       test_num = GET_TEST_NUM(args.arg3);
    uint32_t       status = VAL_ERROR;
    sec_cpu_server_test_t  sec_cpu_server_fn_ptr;

    sec_cpu_server_fn_ptr = (sec_cpu_server_test_t)(test_list[test_num].sec_cpu_server_fn);
    if (sec_cpu_server_fn_ptr == NULL)
    {
       VAL_PANIC("\tInvalid server test function");
    }

    /* Execute server function of given test num */
    sec_cpu_server_fn_ptr = sec_cpu_server_fn_ptr + val_image_load_offset;
    status = sec_cpu_server_fn_ptr(args);

    return status;
}

/**
 *   @brief    - Test entry for non-dispatcher Sec CPU endpoints. This executes
 *               client or server test functions based on dispatcher's command
 *   @param    - void
 *   @return   - void
**/
void val_sec_cpu_wait_for_test_fn_req(void)
{
    ffa_args_t        payload;
    uint32_t          test_run_data;
    uint32_t          status = VAL_ERROR;
    ffa_endpoint_id_t target_id, my_id;

    val_memset(&payload, 0, sizeof(ffa_args_t));

    /* Receive the test_num and client_fn/server_fn to run
     * OR receiver service id for nvm and wd functionality
     */

    val_ffa_msg_wait(&payload);

    while (1)
    {
        if (payload.fid != FFA_MSG_SEND_DIRECT_REQ_32)
        {
            LOG(ERROR, "Invalid fid received, fid=0x%x, error=0x%x", payload.fid, payload.arg2);
            return;
        }

        target_id = SENDER_ID(payload.arg1);
        my_id = RECEIVER_ID(payload.arg1);
        test_run_data = (uint32_t)payload.arg3;

        if (GET_TEST_TYPE(test_run_data) == SERVER_TEST)
        {
            status = val_sec_cpu_execute_server_test_fn(payload);
        }
        else
        {
            //Sec CPU Non Primary VM Client handling
            VAL_PANIC("No Support for Sec CPU non Primary VM Client ");
        }

        /* Send test status back */
        val_memset(&payload, 0, sizeof(ffa_args_t));
        payload.arg1 = ((uint32_t)my_id << 16) | target_id;
        payload.arg3 = status;
        val_ffa_msg_send_direct_resp_32(&payload);
    }
}


/**
 *   @brief    - Retrieve other pe current test status
 *   @param    - mpid , current test number
 *   @return   - test result
**/
uint32_t val_get_multi_pe_test_status(uint64_t mpid, uint32_t test_num)
{
    uint32_t cpu_id = val_get_cpuid(mpid);
    uint32_t mp_test_num = 0;
    uint32_t count = 5;
    uint32_t status = VAL_ERROR;

    val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_current_test_num);
    mp_test_num = g_mp_state.g_current_test_num;

    if (mp_test_num != test_num)
    {
        VAL_PANIC("Inconsistent test_num and g_test_num ");
    }

    /** check and wait for all pe to reach test completion */
    while (1)
    {
        val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_other_pe_test_state);
        if (g_mp_state.g_other_pe_test_state[cpu_id] != VAL_MP_STATE_INPROGRESS || count == 5)
        {
            break;
        }

        /** sleep for 1ms */
        val_sleep(1);
        count--;
    }

    val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_other_pe_test_result);
    val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_other_pe_test_state);

    if (g_mp_state.g_other_pe_test_state[cpu_id] == VAL_MP_STATE_COMPLETE)
    {
        status = g_mp_state.g_other_pe_test_result[cpu_id];

        /* Reset Global MP Test data */
        g_mp_state.g_other_pe_test_state[cpu_id] = VAL_MP_STATE_WAIT;
        g_mp_state.g_other_pe_test_result[cpu_id] = VAL_STATUS_INVALID;
        val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_other_pe_test_result);
        val_dataCacheInvalidateVA((uint64_t)&g_mp_state.g_other_pe_test_state);
    }
    else
    {
        VAL_PANIC("Other-PE test state incomplete ");
    }

    return status;
}
