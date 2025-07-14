/*
 * Copyright (c) 2021-2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "pal_smmuv3_testengine.h"
#include "pal_interfaces.h"
#include "pal_spm_helpers.h"

#define PAGE_SIZE_4K        0x1000
#define F_IDX(n)            (n * FRAME_SIZE)
#define TIME_OUT            0x100000

uint32_t smmuv3_configure_testengine(uint32_t stream_id, uint64_t source, uint64_t dest,
                                     uint64_t size, bool secure)
{
    uint32_t num_frames = (uint32_t)size/PAGE_SIZE_4K;
    uint64_t begin = 0;
    uint64_t end_ctrl;
    uint64_t o_buf;
    uint32_t ssd_ns;
    uint32_t data;
    uint64_t time_out = TIME_OUT;
    uint32_t i;

    /* source and destination address NULL check */
    if (!source || !dest)
        return PAL_ERROR;

    if (num_frames < 1)
        return PAL_ERROR;

    if (secure)
        ssd_ns = 0;
    else
        ssd_ns = 1;

    for (i = 0; i < num_frames; i++)
    {
        begin = source + (i * PAGE_SIZE_4K);
        end_ctrl = begin + (PAGE_SIZE_4K - 1);
        o_buf = dest + (i * PAGE_SIZE_4K);
        /* Initialize the Privileged frame */
        pal_mmio_write32((P_FRAME_BASE + PCTRL + F_IDX(i)), ssd_ns);
        pal_mmio_write32((P_FRAME_BASE + DOWNSTREAM_PORT_INDEX + F_IDX(i)), 0);
        pal_mmio_write32((P_FRAME_BASE + STREAM_ID + F_IDX(i)), stream_id);
        pal_mmio_write32((P_FRAME_BASE + SUBSTREAM_ID + F_IDX(i)), NO_SUBSTREAMID);

        /* Initialize the user frame */
        pal_mmio_write32((U_FRAME_BASE + UCTRL + F_IDX(i)), 0);
        pal_mmio_write32((U_FRAME_BASE + SEED + F_IDX(i)), 0);
        pal_mmio_write64((U_FRAME_BASE + BEGIN + F_IDX(i)), begin);
        pal_mmio_write64((U_FRAME_BASE + END_CTRL + F_IDX(i)), end_ctrl);
        pal_mmio_write32((U_FRAME_BASE + STRIDE + F_IDX(i)), 0x1);
        pal_mmio_write64((U_FRAME_BASE + UDATA + F_IDX(i)), o_buf);
        pal_mmio_write32((U_FRAME_BASE + CMD + F_IDX(i)), ENGINE_MEMCPY);

        while (--time_out)
        {
            data = pal_mmio_read32((U_FRAME_BASE + CMD + F_IDX(i)));

            if (data == ENGINE_MEMCPY)
                continue;

            if (data != ENGINE_HALTED)
            {
                PAL_LOG("\t\tSMMU data transfer failed\n", 0, 0);
                return PAL_ERROR;
            }
            else
                break;
        }

        if (!time_out)
        {
            PAL_LOG("\t\tSMMU test engine timeout\n", 0, 0);
            return PAL_ERROR;
        }

        time_out = TIME_OUT;
        dsbsy();
    }

    if (pal_memcmp((void *)begin, (void *)dest, size))
    {
        PAL_LOG("\t\tSMMU: Data mismatched\n", 0, 0);
        return PAL_ERROR;
    }

    return PAL_SUCCESS;
}
