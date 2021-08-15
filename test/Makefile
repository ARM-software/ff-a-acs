##
 # Copyright (c) 2021, Arm Limited or its affiliates. All rights reserved.
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
                $(TEST_SRC)/direct_messaging/ffa_direct_message_error1/ffa_direct_message_error1_testentry.o

ffa_acs_test-objs += $(TEST_SRC)/indirect_messaging/ffa_msg_send/ffa_msg_send_client.o \
                $(TEST_SRC)/indirect_messaging/ffa_msg_send/ffa_msg_send_server.o \
                $(TEST_SRC)/indirect_messaging/ffa_msg_send/ffa_msg_send_testentry.o \
                $(TEST_SRC)/indirect_messaging/ffa_msg_send_error/ffa_msg_send_error_client.o \
                $(TEST_SRC)/indirect_messaging/ffa_msg_send_error/ffa_msg_send_error_testentry.o 

ffa_acs_test-objs += $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_share/ffa_mem_share_testentry.o \
                $(TEST_SRC)/memory_manage/share_cache_attr/share_cache_attr_client.o \
                $(TEST_SRC)/memory_manage/share_cache_attr/share_cache_attr_server.o \
                $(TEST_SRC)/memory_manage/share_cache_attr/share_cache_attr_testentry.o \
                $(TEST_SRC)/memory_manage/share_device_attr/share_device_attr_client.o \
                $(TEST_SRC)/memory_manage/share_device_attr/share_device_attr_server.o \
                $(TEST_SRC)/memory_manage/share_device_attr/share_device_attr_testentry.o \
                $(TEST_SRC)/memory_manage/share_device_attr/share_device_attr1_client.o \
                $(TEST_SRC)/memory_manage/share_device_attr/share_device_attr1_server.o \
                $(TEST_SRC)/memory_manage/share_device_attr/share_device_attr1_testentry.o \
                $(TEST_SRC)/memory_manage/share_input_error_checks/share_input_error_checks1_client.o \
                $(TEST_SRC)/memory_manage/share_input_error_checks/share_input_error_checks1_testentry.o \
                $(TEST_SRC)/memory_manage/share_input_error_checks/share_input_error_checks_client.o \
                $(TEST_SRC)/memory_manage/share_input_error_checks/share_input_error_checks_testentry.o \
                $(TEST_SRC)/memory_manage/share_invalid_handle_tag/share_invalid_handle_tag_client.o \
                $(TEST_SRC)/memory_manage/share_invalid_handle_tag/share_invalid_handle_tag_server.o \
                $(TEST_SRC)/memory_manage/share_invalid_handle_tag/share_invalid_handle_tag_testentry.o \
                $(TEST_SRC)/memory_manage/share_lower_upper_boundary/share_lower_upper_boundary_32_client.o \
                $(TEST_SRC)/memory_manage/share_lower_upper_boundary/share_lower_upper_boundary_32_server.o \
                $(TEST_SRC)/memory_manage/share_lower_upper_boundary/share_lower_upper_boundary_32_testentry.o \
                $(TEST_SRC)/memory_manage/share_lower_upper_boundary/share_lower_upper_boundary_64_client.o \
                $(TEST_SRC)/memory_manage/share_lower_upper_boundary/share_lower_upper_boundary_64_server.o \
                $(TEST_SRC)/memory_manage/share_lower_upper_boundary/share_lower_upper_boundary_64_testentry.o \
                $(TEST_SRC)/memory_manage/share_relinquish_input_checks/share_relinquish_input_checks_client.o \
                $(TEST_SRC)/memory_manage/share_relinquish_input_checks/share_relinquish_input_checks_server.o \
                $(TEST_SRC)/memory_manage/share_relinquish_input_checks/share_relinquish_input_checks_testentry.o \
                $(TEST_SRC)/memory_manage/share_retrieve_alignment_hint/share_retrieve_align_hint_check_client.o \
                $(TEST_SRC)/memory_manage/share_retrieve_alignment_hint/share_retrieve_align_hint_check_server.o \
                $(TEST_SRC)/memory_manage/share_retrieve_alignment_hint/share_retrieve_align_hint_check_testentry.o \
                $(TEST_SRC)/memory_manage/share_retrieve_input_checks/share_retrieve_input_checks_client.o \
                $(TEST_SRC)/memory_manage/share_retrieve_input_checks/share_retrieve_input_checks_server.o \
                $(TEST_SRC)/memory_manage/share_retrieve_input_checks/share_retrieve_input_checks_testentry.o \
                $(TEST_SRC)/memory_manage/share_retrieve_with_address_range/share_retrieve_with_address_range_client.o \
                $(TEST_SRC)/memory_manage/share_retrieve_with_address_range/share_retrieve_with_address_range_server.o \
                $(TEST_SRC)/memory_manage/share_retrieve_with_address_range/share_retrieve_with_address_range_testentry.o \
                $(TEST_SRC)/memory_manage/share_ro_retrieve_rw/share_ro_retrieve_rw_32_client.o \
                $(TEST_SRC)/memory_manage/share_ro_retrieve_rw/share_ro_retrieve_rw_32_server.o \
                $(TEST_SRC)/memory_manage/share_ro_retrieve_rw/share_ro_retrieve_rw_32_testentry.o \
                $(TEST_SRC)/memory_manage/share_ro_retrieve_rw/share_ro_retrieve_rw_64_client.o \
                $(TEST_SRC)/memory_manage/share_ro_retrieve_rw/share_ro_retrieve_rw_64_server.o \
                $(TEST_SRC)/memory_manage/share_ro_retrieve_rw/share_ro_retrieve_rw_64_testentry.o \
                $(TEST_SRC)/memory_manage/share_rw_retrieve_ro/share_rw_retrieve_ro_client.o \
                $(TEST_SRC)/memory_manage/share_rw_retrieve_ro/share_rw_retrieve_ro_server.o \
                $(TEST_SRC)/memory_manage/share_rw_retrieve_ro/share_rw_retrieve_ro_testentry.o \
                $(TEST_SRC)/memory_manage/share_shareability_attr/share_shareability_attr1_client.o \
                $(TEST_SRC)/memory_manage/share_shareability_attr/share_shareability_attr1_server.o \
                $(TEST_SRC)/memory_manage/share_shareability_attr/share_shareability_attr1_testentry.o \
                $(TEST_SRC)/memory_manage/share_shareability_attr/share_shareability_attr_client.o \
                $(TEST_SRC)/memory_manage/share_shareability_attr/share_shareability_attr_server.o \
                $(TEST_SRC)/memory_manage/share_shareability_attr/share_shareability_attr_testentry.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_1_client.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_1_server.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_1_testentry.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_2_client.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_2_testentry.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_3_client.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_3_server.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_3_testentry.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_4_client.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_4_server.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_4_testentry.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_5_client.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_5_server.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_5_testentry.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_6_client.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_6_server.o \
                $(TEST_SRC)/memory_manage/share_state_machine/share_state_machine_6_testentry.o 

ffa_acs_test-objs += $(TEST_SRC)/memory_manage/ffa_mem_lend/ffa_mem_lend_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_lend/ffa_mem_lend_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_lend/ffa_mem_lend_testentry.o \
                $(TEST_SRC)/memory_manage/lend_input_error_checks/lend_input_error_checks1_client.o \
                $(TEST_SRC)/memory_manage/lend_input_error_checks/lend_input_error_checks1_testentry.o \
                $(TEST_SRC)/memory_manage/lend_input_error_checks/lend_input_error_checks_client.o \
                $(TEST_SRC)/memory_manage/lend_input_error_checks/lend_input_error_checks_testentry.o \
                $(TEST_SRC)/memory_manage/lend_invalid_handle_tag/lend_invalid_handle_tag_client.o \
                $(TEST_SRC)/memory_manage/lend_invalid_handle_tag/lend_invalid_handle_tag_server.o \
                $(TEST_SRC)/memory_manage/lend_invalid_handle_tag/lend_invalid_handle_tag_testentry.o \
                $(TEST_SRC)/memory_manage/lend_mem_access_after_lend/lend_mem_access_after_lend_32_client.o \
                $(TEST_SRC)/memory_manage/lend_mem_access_after_lend/lend_mem_access_after_lend_32_testentry.o \
                $(TEST_SRC)/memory_manage/lend_mem_access_after_lend/lend_mem_access_after_lend_64_client.o \
                $(TEST_SRC)/memory_manage/lend_mem_access_after_lend/lend_mem_access_after_lend_64_testentry.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_alignment_hint/lend_retrieve_align_hint_check_client.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_alignment_hint/lend_retrieve_align_hint_check_server.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_alignment_hint/lend_retrieve_align_hint_check_testentry.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks1_client.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks1_server.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks1_testentry.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks2_client.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks2_server.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks2_testentry.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks3_client.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks3_server.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks3_testentry.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks_client.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks_server.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_input_checks/lend_retrieve_input_checks_testentry.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_mem_access/lend_retrieve_mem_access_32_client.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_mem_access/lend_retrieve_mem_access_32_server.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_mem_access/lend_retrieve_mem_access_32_testentry.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_mem_access/lend_retrieve_mem_access_64_client.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_mem_access/lend_retrieve_mem_access_64_server.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_mem_access/lend_retrieve_mem_access_64_testentry.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_with_address_range/lend_retrieve_with_address_range_client.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_with_address_range/lend_retrieve_with_address_range_server.o \
                $(TEST_SRC)/memory_manage/lend_retrieve_with_address_range/lend_retrieve_with_address_range_testentry.o

ffa_acs_test-objs += $(TEST_SRC)/memory_manage/donate_input_error_checks/donate_input_error_checks_client.o \
                $(TEST_SRC)/memory_manage/donate_input_error_checks/donate_input_error_checks_testentry.o \
                $(TEST_SRC)/memory_manage/donate_invalid_handle_tag/donate_invalid_handle_tag_client.o \
                $(TEST_SRC)/memory_manage/donate_invalid_handle_tag/donate_invalid_handle_tag_server.o \
                $(TEST_SRC)/memory_manage/donate_invalid_handle_tag/donate_invalid_handle_tag_testentry.o \
                $(TEST_SRC)/memory_manage/donate_lower_upper_boundary/donate_lower_upper_boundary_32_client.o \
                $(TEST_SRC)/memory_manage/donate_lower_upper_boundary/donate_lower_upper_boundary_32_server.o \
                $(TEST_SRC)/memory_manage/donate_lower_upper_boundary/donate_lower_upper_boundary_32_testentry.o \
                $(TEST_SRC)/memory_manage/donate_lower_upper_boundary/donate_lower_upper_boundary_64_client.o \
                $(TEST_SRC)/memory_manage/donate_lower_upper_boundary/donate_lower_upper_boundary_64_server.o \
                $(TEST_SRC)/memory_manage/donate_lower_upper_boundary/donate_lower_upper_boundary_64_testentry.o \
                $(TEST_SRC)/memory_manage/donate_retrieve_input_checks/donate_retrieve_input_checks1_client.o \
                $(TEST_SRC)/memory_manage/donate_retrieve_input_checks/donate_retrieve_input_checks1_server.o \
                $(TEST_SRC)/memory_manage/donate_retrieve_input_checks/donate_retrieve_input_checks1_testentry.o \
                $(TEST_SRC)/memory_manage/donate_retrieve_input_checks/donate_retrieve_input_checks2_client.o \
                $(TEST_SRC)/memory_manage/donate_retrieve_input_checks/donate_retrieve_input_checks2_server.o \
                $(TEST_SRC)/memory_manage/donate_retrieve_input_checks/donate_retrieve_input_checks2_testentry.o 

ffa_acs_test-objs += $(TEST_SRC)/memory_manage/relinquish_mem_unmap_check/relinquish_mem_unmap_check_client.o \
                $(TEST_SRC)/memory_manage/relinquish_mem_unmap_check/relinquish_mem_unmap_check_server.o \
                $(TEST_SRC)/memory_manage/relinquish_mem_unmap_check/relinquish_mem_unmap_check_testentry.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_2_client.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_2_server.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_2_testentry.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_3_client.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_3_server.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_3_testentry.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_4_client.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_4_server.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_4_testentry.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_5_client.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_5_server.o \
                $(TEST_SRC)/memory_manage/relinquish_state_machine/relinquish_state_machine_5_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_reclaim/reclaim_input_error_checks_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_reclaim/reclaim_input_error_checks_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_reclaim/reclaim_input_error_checks_testentry.o \
                $(TEST_SRC)/memory_manage/ffa_mem_reclaim/reclaim_zero_flag_client.o \
                $(TEST_SRC)/memory_manage/ffa_mem_reclaim/reclaim_zero_flag_server.o \
                $(TEST_SRC)/memory_manage/ffa_mem_reclaim/reclaim_zero_flag_testentry.o

ffa_acs_test-objs += $(TEST_SRC)/setup_discovery/ffa_features/ffa_features_client.o \
                $(TEST_SRC)/setup_discovery/ffa_features/ffa_features_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_id_get/ffa_id_get_client.o \
                $(TEST_SRC)/setup_discovery/ffa_id_get/ffa_id_get_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_partition_info_get/ffa_partition_info_get_client.o \
                $(TEST_SRC)/setup_discovery/ffa_partition_info_get/ffa_partition_info_get_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_rx_release/ffa_rx_release_client.o \
                $(TEST_SRC)/setup_discovery/ffa_rx_release/ffa_rx_release_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_version/ffa_version_client.o \
                $(TEST_SRC)/setup_discovery/ffa_version/ffa_version_testentry.o \
                $(TEST_SRC)/setup_discovery/ffa_rxtx_map_and_unmap/ffa_rxtx_map_and_unmap_client.o \
                $(TEST_SRC)/setup_discovery/ffa_rxtx_map_and_unmap/ffa_rxtx_map_and_unmap_testentry.o \
                $(TEST_SRC)/setup_discovery/rxtx_exclusive_access/rxtx_exclusive_access_client.o \
                $(TEST_SRC)/setup_discovery/rxtx_exclusive_access/rxtx_exclusive_access_server.o \
                $(TEST_SRC)/setup_discovery/rxtx_exclusive_access/rxtx_exclusive_access_testentry.o \
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

