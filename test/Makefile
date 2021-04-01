##
 # Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.
 #
 # SPDX-License-Identifier: BSD-3-Clause
 #
##

export CROSS_COMPILE ?= aarch64-linux-gnu-
export ARCH ?= arm64

# ACS Make commandline arguments
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
TEST_SRC = $(ACS_DIR)

ACS_INCLUDE = -I$(PWD)/$(ACS_DIR)/../val/inc -I$(PWD)/$(ACS_DIR)/common/ -I$(PWD)/$(ACS_DIR)/../common/inc -I$(PWD)/$(ACS_DIR)/../pal_linux/inc
obj-m += ffa_acs_test.o

ffa_acs_test-objs += $(TEST_SRC)/common/test_database.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_32/ffa_direct_message_32_client.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_32/ffa_direct_message_32_testentry.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_32/ffa_direct_message_32_server.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_64/ffa_direct_message_64_client.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_64/ffa_direct_message_64_testentry.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_64/ffa_direct_message_64_server.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_error/ffa_direct_message_error_client.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_error/ffa_direct_message_error_testentry.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_error1/ffa_direct_message_error1_client.o \
                $(TEST_SRC)/direct_messaging/ffa_direct_message_error1/ffa_direct_message_error1_testentry.o \
                $(TEST_SRC)/indirect_messaging/ffa_msg_send/ffa_msg_send_client.o \
                $(TEST_SRC)/indirect_messaging/ffa_msg_send/ffa_msg_send_server.o \
                $(TEST_SRC)/indirect_messaging/ffa_msg_send/ffa_msg_send_testentry.o \
                $(TEST_SRC)/indirect_messaging/ffa_msg_send_error/ffa_msg_send_error_client.o \
                $(TEST_SRC)/indirect_messaging/ffa_msg_send_error/ffa_msg_send_error_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_donate/ffa_mem_donate_mmio_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_donate/ffa_mem_donate_mmio_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_lend/ffa_mem_lend_mmio_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_lend/ffa_mem_lend_mmio_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_mmio_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_tag_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_handle_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_mmio_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_tag_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_handle_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_handle_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_tag_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share_read_only_chk/ffa_mem_share_32_read_only_chk_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share_read_only_chk/ffa_mem_share_32_read_only_chk_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share_read_only_chk/ffa_mem_share_32_read_only_chk_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share_read_only_chk/ffa_mem_share_64_read_only_chk_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share_read_only_chk/ffa_mem_share_64_read_only_chk_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share_read_only_chk/ffa_mem_share_64_read_only_chk_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share_retrieve/ffa_mem_share_retrieve_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share_retrieve/ffa_mem_share_retrieve_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share_retrieve/ffa_mem_share_retrieve_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_features/ffa_features_client.o \
                $(TEST_SRC)/setup_discovery/ffa_features/ffa_features_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_id_get/ffa_id_get_client.o \
                $(TEST_SRC)/setup_discovery/ffa_id_get/ffa_id_get_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_partition_info_get/ffa_partition_info_get_client.o \
                $(TEST_SRC)/setup_discovery/ffa_partition_info_get/ffa_partition_info_get_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_rx_release/ffa_rx_release_client.o \
                $(TEST_SRC)/setup_discovery/ffa_rx_release/ffa_rx_release_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_version/ffa_version_client.o \
                $(TEST_SRC)/setup_discovery/ffa_version/ffa_version_testentry.o \
                $(TEST_SRC)/setup_discovery/mp_execution_contexts/mp_execution_contexts_client.o \
                $(TEST_SRC)/setup_discovery/mp_execution_contexts/mp_execution_contexts_testentry.o \
                $(TEST_SRC)/setup_discovery/up_migrate_capable/up_migrate_capable_client.o \
                $(TEST_SRC)/setup_discovery/up_migrate_capable/up_migrate_capable_server.o \
                $(TEST_SRC)/setup_discovery/up_migrate_capable/up_migrate_capable_testentry.o

ccflags-y=$(ACS_MACROS) $(ACS_INCLUDE) -Wall -Werror

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

