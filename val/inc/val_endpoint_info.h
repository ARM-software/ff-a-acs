/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_ENPOINT_INFO_H_
#define _VAL_ENPOINT_INFO_H_

#include "val_framework.h"
#include "val_interfaces.h"
#include "val_ffa.h"

#define VAL_ENDPOINT_INFO_SIZE      32
#define VAL_PARTITION_NOT_PRESENT   0x0
#define VAL_PARTITION_PRESENT       0xF
#define VAL_PARTITION_SECURE        0xF
#define VAL_PARTITION_NONSECURE     0x0

typedef struct val_endpoint_info {
    char                name[5];
    union {
        uint8_t partition_status; // Full byte access if needed
        struct {
            uint8_t is_valid    : 4;
            uint8_t is_secure   : 4;
        };
    };
    ffa_endpoint_id_t   id;
    uint8_t             tg0;
    uint8_t             el_info;
    uint16_t            ec_count;
    uint32_t            ep_properties;
    uint32_t            uuid[4];
} val_endpoint_info_t;

ffa_endpoint_id_t val_get_endpoint_id(uint32_t logical_id);
ffa_endpoint_id_t val_get_endpoint_logical_id(ffa_endpoint_id_t endpoint_id);
ffa_endpoint_id_t val_get_curr_endpoint_id(void);
ffa_endpoint_id_t val_get_curr_endpoint_logical_id(void);
char *val_get_curr_endpoint_name(void);
char *val_get_endpoint_name(uint32_t logical_id);
uint64_t val_get_curr_endpoint_tt_tg0(void);
void val_set_endpoint_tt_tg0(uint32_t logical_id, uint8_t tg0);
uint8_t val_get_curr_endpoint_el_info(void);
uint8_t val_get_endpoint_el_info(uint32_t logical_id);
val_endpoint_info_t *val_get_endpoint_info(void);
uint32_t val_assign_tg0_to_endpoint(void);
uint32_t val_curr_endpoint_page_size(void);
uint32_t val_get_endpoint_info_table_count(void);
uint32_t val_get_secure_partition_count(void);
uint32_t val_is_partition_valid(uint32_t logical_id);
#endif /* _VAL_ENPOINT_INFO_H_ */
