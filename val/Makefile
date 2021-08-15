##
 # Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
 #
 # SPDX-License-Identifier: BSD-3-Clause
 #
##

export CROSS_COMPILE ?= aarch64-linux-gnu-
export ARCH ?= arm64

# ACS CMake commandline arguments
VERBOSITY ?= 3
PLATFORM_NS_HYPERVISOR_PRESENT ?=  0
PLATFORM_SPMC_EL ?= 2
PLATFORM_SP_EL   ?= 1
SUITE            ?= all

ACS_MACROS += -DVM1_COMPILE -DTARGET_LINUX
ACS_MACROS += -DVERBOSITY=$(VERBOSITY)
ACS_MACROS += -DPLATFORM_NS_HYPERVISOR_PRESENT=$(PLATFORM_NS_HYPERVISOR_PRESENT)
ACS_MACROS += -DPLATFORM_SPMC_EL=$(PLATFORM_SPMC_EL)
ACS_MACROS += -DPLATFORM_SP_EL=$(PLATFORM_SP_EL)
ACS_MACROS += -DSUITE=$(SUITE)

#since we have copied the files locally
ACS_DIR ?= .
VAL_SRC = $(ACS_DIR)/src

ACS_INCLUDE = -I$(PWD)/$(ACS_DIR)/inc -I$(PWD)/$(ACS_DIR)/../common/inc -I$(PWD)/$(ACS_DIR)/../pal_linux/inc
obj-m += ffa_acs_val.o

ffa_acs_val-objs += $(VAL_SRC)/val_main.o  \
                    $(VAL_SRC)/val_endpoint_info.o \
                    $(VAL_SRC)/val_framework.o \
                    $(VAL_SRC)/val_ffa_abi.o \
                    $(VAL_SRC)/val_ffa_helpers.o \
                    $(VAL_SRC)/val_misc.o \
                    $(VAL_SRC)/val_vcpu_setup.o \
                    $(VAL_SRC)/val_shemaphore.o \
                    $(VAL_SRC)/val_exceptions.o \
                    $(VAL_SRC)/val_test_dispatch.o \
                    $(VAL_SRC)/val_irq.o \
                    $(VAL_SRC)/aarch64/val_syscalls.o \
                    $(VAL_SRC)/aarch64/val_sysreg.o \
                    $(VAL_SRC)/aarch64/val_vtable.o

ccflags-y=$(ACS_MACROS) $(ACS_INCLUDE) -Wall -Werror
KBUILD_AFLAGS := $(ACS_MACROS) $(ACS_INCLUDE) -D__ASSEMBLY__

all:
ifeq ($(KERNEL_SRC),)
	echo "  KERNEL_SRC variable should be set to kernel path "
	exit 1
else
	echo "Kernel source is set to $(KERNEL_SRC)"
endif

	make -C $(KERNEL_SRC) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	make -C $(KERNEL_SRC) M=$(PWD) clean
	find $(ACS_DIR) -type f -name "*.o" -delete
	find $(ACS_DIR) -type f -name "*.o.cmd" -delete

