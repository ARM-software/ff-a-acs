
# Arm FF-A ACS Porting Guide
-----------------------------

## Introduction
The architecture test suite contains a *Platform Abstraction Layer* (PAL) which abstracts platform-specific information from the tests. You must implement and port the PAL interface functions available in **platform/** directory to your target platform. Create and update the target configuration file to match the details of this target platform.

This document provides the porting steps and the list of PAL APIs.

## Steps to add new target (or Platform)
  1. Create new directories in **platform/pal_baremetal/<platform_name>** and **platform/manifest/<platform_name>/**. For reference, see the existing platform tgt_tfa_fvp directory.
  2. Execute `cp -rf platform/pal_baremetal/tgt_tfa_fvp/ platform/pal_baremetal/<platform_name>/` and `cp -rf platform/manifest/tgt_tfa_fvp platform/manifest/<platform_name>/`
  3. Update **platform/pal_baremetal/<platform_name>/pal_config_def.h** with your target platform details.
  4. Refer section **List of PAL APIs** file to view the list of PAL APIs that must be ported for your target platform. The reference implementation for these APIs are available in **platform/pal_baremetal/tgt_tfa_fvp/pal_\*\.c** files. You can reuse the code if it works for your platform. Otherwise, port them for your platform-specific peripherals.
  5. Update **platform/pal_baremetal/<platform_name>/pal.cmake** appropriately to select the correct instances of PAL files for compilation.
  6. Update the platform information available in test endpoint manifest files located in **platform/manifest/<platform_name>/** directory with your platform information. Refer **Endpoint manifest requirement** section for more detail.

## Endpoint manifest requirement
As defined in the Arm FF-A specification, partition managers are expected to use partition resource parameters defined in the partition manifest for setting up the partition. The partition's manifest file must provide fields specifying partition properties, memory regions, devices, partition boot protocol of the partition.

The following aspects of the partition manifest are implementation defined:
- Format of manifest file
- Time of creation of manifest file. This could be at: Build time, Boot time or Combination of both.
- Mechanism used by the Hypervisor and SPM to obtain the information in the manifest file and interpret its contents

Arm FF-A ACS also provides the partition manifests for its test partitions with manifest format supported by TF-A reference platform. These use the [dts format](https://trustedfirmware-a.readthedocs.io/en/latest/components/psa-ffa-manifest-binding.html#psa-ff-a-manifest-binding-to-device-tree) which can be bind to device tree. This helps smooth out of box testing with TF-A platform. If your platform supports different manifest format, this document will help to map these reference manifest files to your target platform. Since reference manifests are specific to TF-A manifest format, not all parameters are required to populate for your platform. Therefore, This document describes following minimum list of manifest parameters that the ACS partitions are rely on for its partitions setup and those must be mapped and populated for your platform:

- FF-A version
- UUID
- Partition ID
- Number of execution contexts
- Run-Time EL
- Execution state
- Messaging method
- Memory region - Page count
- Memory region - Attributes
- Device region - Physical base address
- Device region - Page count
- Device region - Attributes

The platform details provided in the manifest files must match with details provided in the pal_config_def.h file. Also for better test coverage, test suite recommonds that partner populates the values of some of these fields as per the endpoint properties specified in platform/common/inc/pal_endpoint_info.h files. The example of such fields are number of execution contexts and messaging types.

## Peripheral requirement
- One secure UART must be assigned to test endpoint SP1 for printing its test messages
- One non-secure UART must be assigned to test endpoint VM1 for printing its test messages.
- Other test endpoint relies on system provided print functionality using pal_uart_putc_hypcall function.
- One Watchdog timer must be assigned to SP1 to help system recovery from any fatal error conditions
- 64KB Non-volatile memory assigned to SP1 for preserving test data over watchdog timer reset. Each byte of this region must be initialised to FF at power on reset.

## List of PAL APIs

Since the test suite is agnostic to various system targets, you must port the following PAL APIs before building the tests. <br />

| No | Prototype                                 | Description                                  | Parameters                            |
|----|-----------------------------------------------------------------------------------|-----------------------------------------------------------------------------------|----------------------------------------------------------|
| 1 |  uint32_t pal_printf(const char *msg, uint64_t data1, uint64_t data2); |  This function prints the given string and data onto the uart| msg: Input String <br /> data1: Value for first format specifier <br /> data2: Value for second format specifier <br /> Return: SUCCESS(0)/FAILURE(any positive number) |
| 2 | uint32_t pal_nvm_write(uint32_t offset, void *buffer, size_t size); | Writes into given non-volatile address | offset: Offset into nvmem <br />buffer: Pointer to source address<br /> size: Number of bytes<br /> Return: SUCCESS/FAILURE|
| 3 | uint32_t pal_nvm_read(uint32_t offset, void *buffer, size_t size); | Reads from given non-volatile address | offset: Offset into nvmem <br />buffer: Pointer to destination address<br /> size: Number of bytes<br /> Return: SUCCESS/FAILURE|
| 4 | uint32_t pal_watchdog_enable(void); | Initializes and enable the hardware watchdog timer | Input: void <br /> Return: SUCCESS/FAILURE|
| 5 | uint32_t pal_watchdog_disable(void); | Disable the hardware watchdog timer | Input: void <br /> Return: SUCCESS/FAILURE|
| 6 | uint32_t pal_get_endpoint_device_map(void **region_list, size_t *no_of_mem_regions); | Returns the base address of endpoint device map region block. This data is used to map device regions into endpoint translation table. | region_list: Populates the base address of device map region block <br /> no_of_mem_regions: Populates number of device regions assigned to endpoint<br /> Return: SUCCESS/FAILURE|
| 7 | uint32_t pal_get_no_of_cpus(void);|Returns number of cpus in the system. | Input: Void <br /> Return: Number of cpus|
| 8 | uint32_t pal_get_cpuid(uint64_t mpid); | Convert mpid to logical cpu number | Input: mpid value <br /> Return:Logical cpu number|
| 9 | uint64_t pal_get_mpid(uint32_t cpuid);| Return mpid value of given logical cpu index| Input: Logical cpu index <br /> Return: Mpidr value|
| 10 | uint32_t pal_power_on_cpu(uint64_t mpid);| Power up the given core| mpidr: Mpid value of the core <br /> Return: SUCCESS/FAILURE |
| 11 | uint32_t pal_power_off_cpu(void);| Power down the calling core.| Input: void <br /> Return: The call does not return when successful. Otherwise, it returns PAL_ERROR. |
| 12 | uint32_t pal_terminate_simulation(void);| Terminates the simulation at the end of all tests completion.| Input: void <br /> Return:SUCCESS/FAILURE|
| 13 | void *pal_mem_virt_to_phys(void *va);| Returns the physical address of the input virtual address| va: Virtual address of the memory to be converted <br /> Return: Physical addresss|
| 14 | uint32_t pal_smmu_device_configure(uint32_t stream_id, uint64_t source, uint64_t dest, uint64_t size, bool secure);| This function configures SMMU upstream device and initiates DMA transfer from source address to destination.| stream_id: stream_id of the device. <br />source: Source address. <br />dest: Destination address <br />size: Size of the memory <br />secure: device attribute secure/non-secure <br /> Return: SUCCESS/FAILURE|
| 15 | void pal_irq_setup(void); | Initialise the irq vector table. | input: void <br /> return: void |
| 16 | int pal_irq_handler_dispatcher(void); | Generic handler called upon reception of an IRQ. This function acknowledges the interrupt, calls the user-defined handler. If one has been registered then marks the processing of the interrupt as complete. | input: void <br /> return: SUCCESS/FAILURE |
| 17 | void pal_irq_enable(int irq_num, uint8_t irq_priority); | Enable interrupt #irq_num for the calling core. | irq_num: irq number. <br /> irq_priority:  irq priority value. <br /> return: void |
| 18 | void pal_irq_disable(int irq_num); | Disable interrupt #irq_num for the calling core. | irq_num: irq number. <br /> return: void |
| 19 | int pal_irq_register_handler(int irq_num, void *irq_handler); | Register an interrupt handler for a given interrupt number. This will fail if there is already an interrupt handler registered for the same interrupt.| irq_num: irq number. <br /> irq_handler:  irq handler pointer <br /> return: Return 0 on success, a negative value otherwise |
| 20 | int pal_irq_unregister_handler(int irq_num); | Unregister an interrupt handler for a given interrupt number. This will fail if there is no an interrupt handler registered for the same interrupt.| irq_num: irq number. <br /> return: Return 0 on success, a negative value otherwise |
| 21 | void pal_send_sgi(int sgi_id, unsigned int core_pos); | Send an SGI to a given core. | sgi_id: SGI interrupt number. <br />core_pos:  CPU core number. <br /> return: void |

## License

Arm FF-A ACS is distributed under BSD-3-Clause License.

--------------

*Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.*
