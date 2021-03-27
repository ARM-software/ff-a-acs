/*
 * Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _VAL_ENPOINT_INFO_H_
#define _VAL_ENPOINT_INFO_H_

#include "val_framework.h"
#include "val_interfaces.h"
#include "val_ffa.h"

typedef struct val_endpoint_info {
    char                name[5];
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
#endif /* _VAL_ENPOINT_INFO_H_ */
