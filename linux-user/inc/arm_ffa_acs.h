/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _LINUX_ARM_FFA_ACS_H
#define _LINUX_ARM_FFA_ACS_H

/*
 * argument for a FFA_ACS_IOC_CALL ioctl
 * This must be kept in sync with struct arm_smccc_1_2_regs
 */
struct ffa_acs_smc {
	/* in/out table of values for registers x0-x17 */
	uint64_t regs[18];
};

/*
 * FFA_ACS_IOC_CALL - Execute a FFA call
 *
 * This IOCTL will do an smc/hvc call with the registers x0-x17
 * set using the values passed in the struct ffa_acs_smc.
 * The values contains in x0-x17 are returned back in the struct ffa_acs_smc.
 */
#define FFA_ACS_IOC_CALL	_IOWR('A', 1, struct ffa_acs_smc)

/*
 * argument for FFA_ACS_IOC_ISR ioctl
 */
struct ffa_acs_irq {
	/* out irq number received by the FF-A driver */
	int irq;
	/* out core number on which the irq was received */
	unsigned int processor_id;
};

/*
 * FFA_ACS_IOC_ISR - Wait for the next interrupt
 *
 * This IOCTL will block the caller until an interrupt is received.
 * This will return the interrupt information in the ffa_acs_irq.
 *
 * It is the responsibility of the caller to use FFA notification calls
 * to retrieve the information on the pending notifications.
 *
 * TODO: should we add a timeout parameter ? this could replace the
 * watchdog.
 */
#define FFA_ACS_IOC_ISR	 _IOW('A', 2, struct ffa_acs_irq)

/*
 * FFA_ACS_IOC_MEM_ALLOC - Allocate some physical memory
 *
 * This IOCTL can be used to allocate some memory in the kernel that can be
 * used by the application for exchanges with other endpoints (RX/TX buffers or
 * create shared mappings for example).
 * The uint64_t input argument must contain a size of memory to allocate. This
 * size must be a multiple of the system page size.
 * The call will returned in the uint64_t argument the physical address of the
 * allocated memory.
 * Once memory has been allocated, mmap can be used to map the memory in the
 * application address space.
 */
#define FFA_ACS_IOC_MEM_ALLOC   _IOWR('A', 3, uint64_t)

/*
 * FFA_ACS_IOC_MEM_FREE - Free previously allocated physical memory
 *
 * This IOCTL will free the memory that was allocated using
 * FFA_ACS_IOC_MEM_ALLOC.
 * The memory should be first unmap from the application using munmap.
 * TODO: check what would happen if memory is not munmap first and if not
 * doing it could be used to allocate several buffers.
 */
#define FFA_ACS_IOC_MEM_FREE	_IO('A', 4)

#endif /* _LINUX_ARM_FFA_ACS_H */
