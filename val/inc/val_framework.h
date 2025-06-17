/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_FRAMEWORK_H_
#define _VAL_FRAMEWORK_H_

#include "val.h"
#include "val_misc.h"
#include "val_ffa.h"

void val_main(void);
uint32_t val_report_status(uint32_t test_num);
void val_set_status(uint32_t status);
uint32_t val_get_status(void);
void val_test_init(uint32_t test_num);
void val_test_exit(void);
uint32_t val_get_last_run_test_info(test_info_t *test_info);
uint32_t val_nvm_write(uint32_t offset, void *buffer, size_t size);
uint32_t val_nvm_read(uint32_t offset, void *buffer, size_t size);
uint32_t val_watchdog_enable(void);
uint32_t val_watchdog_disable(void);
void val_set_reboot_flag(void);
void val_reset_reboot_flag(void);
void val_reprogram_watchdog(void);
uint32_t val_smmu_device_configure(uint32_t stream_id, uint64_t source, uint64_t dest,
                                     uint64_t size, bool secure);
uint32_t val_check_ep_compile_status(uint32_t client_logical_id, uint32_t server_logical_id);
void val_send_sync_ep_info(void);
void val_sync_ep_info_service(ffa_args_t *args);
void val_ep_info_relayer_sync(void);

#endif /* _VAL_FRAMEWORK_H_ */

