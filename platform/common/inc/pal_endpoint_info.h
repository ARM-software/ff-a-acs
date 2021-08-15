/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_ENPOINT_INFO_H_
#define _PAL_ENPOINT_INFO_H_

#include "pal_interfaces.h"

#define FFA_RECEIPT_DIRECT_REQUEST_SUPPORT 0x1
#define FFA_DIRECT_REQUEST_SEND            0x2
#define FFA_INDIRECT_MESSAGE_SUPPORT       0x4
#define FFA_MESSAGING_MASK                 0x7

#define PLATFORM_VM1_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND | \
                                    FFA_INDIRECT_MESSAGE_SUPPORT)
#define PLATFORM_VM1_UUID {0, 0, 0, 0}

#define PLATFORM_VM2_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND | \
                                    FFA_INDIRECT_MESSAGE_SUPPORT)
#define PLATFORM_VM2_UUID {0, 0, 0, 1}

#define PLATFORM_VM3_EP_PROPERTIES (FFA_INDIRECT_MESSAGE_SUPPORT)
#define PLATFORM_VM3_UUID {0, 0, 0, 2}

#define PLATFORM_SP1_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND)
#define PLATFORM_SP1_UUID {0xb4b5671e, 0x4a904fe1, 0xb81ffb13, 0xdae1dacb}

#define PLATFORM_SP2_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND)
#define PLATFORM_SP2_UUID {0xd1582309, 0xf02347b9, 0x827c4464, 0xf5578fc8}

#define PLATFORM_SP3_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND)
#define PLATFORM_SP3_UUID {0x79b55c73, 0x1d8c44b9, 0x859361e1, 0x770ad8d2}

/*
 * Execution contexts assignment for test endpoints:
 * VM1 - MP endpoint
 * SP1, SP2 - MP endpoint if it is implemented at EL1. Otherwise UP endpoint.
 * All other endpoints are UP endpoint.
 * MP endpoint must have execution contexts equal to number of cpus in the system.
 * */

#define PLATFORM_VM1_EC_COUNT PLATFORM_NO_OF_CPUS
#define PLATFORM_VM2_EC_COUNT 0x1
#define PLATFORM_VM3_EC_COUNT 0x1
#if (PLATFORM_SP_EL == 0)
#define PLATFORM_SP1_EC_COUNT 0x1
#define PLATFORM_SP2_EC_COUNT 0x1
#else
#define PLATFORM_SP1_EC_COUNT PLATFORM_NO_OF_CPUS
#define PLATFORM_SP2_EC_COUNT PLATFORM_NO_OF_CPUS
#endif
#define PLATFORM_SP3_EC_COUNT 0x1

#endif /* _PAL_ENDPOINT_INFO_H_ */
