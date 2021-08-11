/*
 * Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PAL_SMMUV3_TESTENGINE_H_
#define _PAL_SMMUV3_TESTENGINE_H_

#include "pal_interfaces.h"

/* The register file consists of two pages, each of which is 64 KiB
 * Page 0 : 'User' page
 *      1 : 'Privileged' page
 *
 * Each page consists of a number of frames, each of which corresponds to an independent
 * thread of work.
 * Frame 0 in the User page corresponds to Frame 0 in the Priv page.
 * Each Frame is 128 bytes long.
 */

/* PCTRL bit 0 -- ssd-ns -- 1 if the stream belongs to the non-secure world.
 *                          0 if the stream belongs to the secure world
 */

/* The engine memcpy's from region [begin, end_incl] to address udata[0].
 *
 * If stride is 0 then ENGINE_ERROR is produced, udata[2] contains the error
 * address.  No MSI is generated.
 *
 * If stride is 1 then this is a normal memcpy(). If stride is larger then
 * not all the data will be copied.
 *
 * The order and size of the transactions used are determined randomly using
 * seed. If seed is:
 *       0  -- do them from lowest address to highest address
 *      ~0u -- do them in reverse order
 *       otherwise use the value as a seed to do them in random order
 *       The ability to do them in a  non-random order means that we stand a
 *       chance of getting merged event records.
 *
 * This models a work-load to start with some reads and then do some writes.
 *   ENGINE_MEMCPY = 2.
 */

/* Poll 'cmd' until not being ENGINE_MEMCPY:
 *   - ENGINE_FRAME_MISCONFIGURED
 *   - ENGINE_ERROR
 *   - ENGINE_NO_FRAME
 *   - ENGINE_HALTED -- The engine is halted. All others are errors of one kind
 *     or another.
 */

/* Privileged page base address */
#define P_FRAME_BASE 0x2bff0000
/* User page base address */
#define U_FRAME_BASE 0x2bfe0000
/* Each frame is 128 bytes long */
#define FRAME_SIZE 0x80

/* Privileged frame fields */
#define PCTRL 0x0
#define DOWNSTREAM_PORT_INDEX 0x4
#define STREAM_ID 0x8
#define SUBSTREAM_ID 0xC

/* User frame fields */
#define CMD 0x0
#define UCTRL 0x4
#define SEED 0x24
#define BEGIN 0x28
#define END_CTRL 0x30
#define STRIDE 0x38
#define UDATA 0x40

#define NO_SUBSTREAMID  0xFFFFFFFF

enum cmd_t
{
    /* The frame was misconfigured. */
    ENGINE_FRAME_MISCONFIGURED = ~0u - 1,

    /* The engine encountered an error (downstream transaction aborted). */
    ENGINE_ERROR  = ~0u,

    /* This frame is unimplemented or in use by the secure world.
     *
     * A user _can_ write this to cmd and it will be considered to be ENGINE_HALTED.
     */
    ENGINE_NO_FRAME = 0,

    /* The engine is halted. */
    ENGINE_HALTED = 1,

    /* This initiates memory transfer from source to destination */
    ENGINE_MEMCPY = 2
};

/* Configures SMMU test engine and initiates DMA transfer from source address to destination */
uint32_t smmuv3_configure_testengine(uint32_t stream_id, uint64_t source, uint64_t dest,
                                     uint64_t size, bool secure);

#endif /* _PAL_SMMUV3_TESTENGINE_H_ */
