/*
 * Copyright (c) 2021-2024, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_ENPOINT_INFO_H_
#define _PAL_ENPOINT_INFO_H_

#include "pal_interfaces.h"


#define FFA_RECEIPT_DIRECT_REQUEST_SUPPORT     0x1
#define FFA_DIRECT_REQUEST_SEND                0x2
#define FFA_INDIRECT_MESSAGE_SUPPORT           0x4
#define FFA_NOTIFICATION_SUPPORT               0x8
#define FFA_MESSAGING_MASK                     0x7

#if (PLATFORM_FFA_V >= FFA_V_1_1)
#define FFA_PARTITION_INFO_FLAG_RETCOUNT       0x00000001
#define FFA_PARTITION_INFO_FLAG_RETDESC        0x00000000
#endif

#if (PLATFORM_FFA_V >= FFA_V_1_1)
#define FFA_PARTITION_ID_PE_EP_ID              0x00
#define FFA_PARTITION_ID_SEPID_INDEP           0x10
#define FFA_PARTITION_ID_SEPID_DEP             0x20
#define FFA_PARTITION_ID_AUX_ID                0x30
#define FFA_HYP_VM_CREATE_INFO                 0x40
#define FFA_HYP_VM_DESTROY_INFO                0x80
#define FFA_PARTITION_EXEC_STATE_ARCH64        0x100
#endif

#if (PLATFORM_FFA_V >= FFA_V_1_2)
#define FFA_RECEIPT_DIRECT_REQUEST2_SUPPORT    0x200
#define FFA_DIRECT_REQUEST2_SEND               0x400
#else
#define FFA_RECEIPT_DIRECT_REQUEST2_SUPPORT    0x0
#define FFA_DIRECT_REQUEST2_SEND               0x0
#endif


#if (PLATFORM_FFA_V == FFA_V_1_0)
#define PLATFORM_VM1_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND            | \
                                    FFA_INDIRECT_MESSAGE_SUPPORT)
#define PLATFORM_VM1_UUID {0, 0, 0, 0}

#define PLATFORM_VM2_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND            | \
                                    FFA_INDIRECT_MESSAGE_SUPPORT)
#define PLATFORM_VM2_UUID {0, 0, 0, 1}

#define PLATFORM_VM3_EP_PROPERTIES (FFA_INDIRECT_MESSAGE_SUPPORT)
#define PLATFORM_VM3_UUID {0, 0, 0, 2}


#define PLATFORM_SP1_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND)
#define PLATFORM_SP2_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND)
#define PLATFORM_SP3_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT | \
                                    FFA_DIRECT_REQUEST_SEND)
#else
#define PLATFORM_VM1_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT  | \
                                    FFA_DIRECT_REQUEST_SEND             | \
                                    FFA_INDIRECT_MESSAGE_SUPPORT        | \
                                    FFA_PARTITION_EXEC_STATE_ARCH64     | \
                                    FFA_RECEIPT_DIRECT_REQUEST2_SUPPORT | \
                                    FFA_DIRECT_REQUEST2_SEND)
#define PLATFORM_VM1_UUID {0, 0, 0, 0}

#define PLATFORM_VM2_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT  | \
                                    FFA_DIRECT_REQUEST_SEND             | \
                                    FFA_INDIRECT_MESSAGE_SUPPORT        | \
                                    FFA_PARTITION_EXEC_STATE_ARCH64     | \
                                    FFA_RECEIPT_DIRECT_REQUEST2_SUPPORT | \
                                    FFA_DIRECT_REQUEST2_SEND)
#define PLATFORM_VM2_UUID {0, 0, 0, 1}

#define PLATFORM_VM3_EP_PROPERTIES (FFA_INDIRECT_MESSAGE_SUPPORT | \
                                    FFA_PARTITION_EXEC_STATE_ARCH64)
#define PLATFORM_VM3_UUID {0, 0, 0, 2}

#define PLATFORM_SP1_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT  | \
                                    FFA_INDIRECT_MESSAGE_SUPPORT        | \
                                    FFA_NOTIFICATION_SUPPORT            | \
                                    FFA_DIRECT_REQUEST_SEND             | \
                                    FFA_PARTITION_EXEC_STATE_ARCH64     | \
                                    FFA_RECEIPT_DIRECT_REQUEST2_SUPPORT | \
                                    FFA_DIRECT_REQUEST2_SEND)

#define PLATFORM_SP2_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT  | \
                                    FFA_INDIRECT_MESSAGE_SUPPORT        | \
                                    FFA_NOTIFICATION_SUPPORT            | \
                                    FFA_DIRECT_REQUEST_SEND             | \
                                    FFA_PARTITION_EXEC_STATE_ARCH64     | \
                                    FFA_RECEIPT_DIRECT_REQUEST2_SUPPORT | \
                                    FFA_DIRECT_REQUEST2_SEND)

#define PLATFORM_SP3_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT  | \
                                    FFA_NOTIFICATION_SUPPORT            | \
                                    FFA_DIRECT_REQUEST_SEND             | \
                                    FFA_PARTITION_EXEC_STATE_ARCH64     | \
                                    FFA_RECEIPT_DIRECT_REQUEST2_SUPPORT | \
                                    FFA_DIRECT_REQUEST2_SEND)

#define PLATFORM_SP4_EP_PROPERTIES (FFA_RECEIPT_DIRECT_REQUEST_SUPPORT  | \
                                    FFA_NOTIFICATION_SUPPORT            | \
                                    FFA_DIRECT_REQUEST_SEND             | \
                                    FFA_PARTITION_EXEC_STATE_ARCH64     | \
                                    FFA_RECEIPT_DIRECT_REQUEST2_SUPPORT | \
                                    FFA_DIRECT_REQUEST2_SEND)
#endif
#define PLATFORM_SP1_UUID {0x1e67b5b4, 0xe14f904a, 0x13fb1fb8, 0xcbdae1da}

#define PLATFORM_SP2_UUID {0x092358d1, 0xb94723f0, 0x64447c82, 0xc88f57f5}

#define PLATFORM_SP3_UUID {0x735cb579, 0xb9448c1d, 0xe1619385, 0xd2d80a77}

#define PLATFORM_SP4_UUID {0x2658cda4, 0xcf6713e1, 0x49cd10f9, 0x31ef6813}

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
#define PLATFORM_SP4_EC_COUNT 0x1

#endif /* _PAL_ENDPOINT_INFO_H_ */
