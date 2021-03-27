
# Architecture Compliance Suite for Arm Firmware Framework for Armv8-A

## Arm Firmware Framework for Armv8-A

*Arm Firmware Framework for Armv8-A (FF-A)* describes a software architecture that achieves the following goals:

1. Leverages the virtualization extension to isolate software images provided by an ecosystem of vendors from each other
2. Describes interfaces that standardize communication between the various software images. This includes communication between images in the Secure world and Normal world.

The FF-A also goes beyond the above stated goals to ensure that the interfaces can be used to standardize communication:
- In the absence of the Virtualization Extensions in the Secure world. This provides a migration path for existing Secure world software images to a system that implements the Virtualization Extension in the Secure state.
- Between VMs managed by a Hypervisor in the Normal world. The Virtualization Extensions in the Secure state mirrors its counterpart in the Non-secure state. So, a Hypervisor could use the firmware framework interfaces to enable communication between VMs it manages.

The following are the main components of FF-A:
- A Partition Manager (PM) which manages partitions. It is the Hypervisor in Normal world and the Secure Partition Manager (SPM) in Secure world.
- One or more partitions that are sandboxes created by the PM. These could be VMs in Normal world or Secure world. The VMs in Secure world are called Secure Partitions (SP). In this document, the term endpoint is used interchangeably with the term partition.
- Application Binary Interfaces (ABIs) that partitions can invoke to communicate with other partitions.
- A partition manifest that describes the system resources a partition needs, services that a partition implements, and other attributes of the partition that governs its runtime behavior.

For more information, download the [Arm FF-A Specification](https://developer.arm.com/docs/den0077/latest)

### Architecture Compliance Suite

The Architecture Compliance Suite (ACS) is a set of examples of the invariant behaviors that are specified by the Arm FF-A specification. Use this suite to verify whether these behaviors are implemented correctly in your system. This suite contains self-checking and portable C and assembly based tests with directed stimulus. The tests are available as open source. The tests and the corresponding abstraction layers are available with an BSD-3-Clause License allowing for external contribution.

This suite is not a substitute for design verification. To review the test logs, you can contact Arm directly through their partner managers.

See [Validation Methodology](./docs/Arm_FF_A_ACS_Validation_Methodology.pdf) document for more information on Architecture Compliance Suite.

## This release
- Release Version - v0.5
- Code Quality: Alpha - Please use this opportunity to suggest enhancements and point out errors.
- The tests are written for Arm FF-A 1.0 specification version.
- For information about the test coverage scenarios that are implemented in the current release of ACS and the scenarios that are planned for the future releases, see [Testcase checklist](./docs/testcase_checklist.md).

## Github branch
- To pick up the release version of the code, checkout the release branch.
- To get the latest version of the code with bug fixes and new features, use the main branch.

## Software Requirements

Following tools are required to build the ACS: <br />
- Host Operating System     : Ubuntu 18.04
- Git to download ACS repository
- CMake 3.17
- Cross-compiler toolchain supporting AArch64 target : GCC >= 9.2-2019.12 can be downloaded from [Arm Developer website](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads)

**Note** : ACS can also be built using Arm Compiler 6 (Verified with 6.12) or Clang (Verified with 8.0.0) cross compiler toolchain. To compile and assemble sources using these toolchains, set -DCC=<compiler-path> with path pointing to the armclang or clang binary. Note that ACS still needs the GNU linker for generating the test binaries regardless of chosen toolchain.

## Download source

To download the main branch of the repository, type the following command: <br />
~~~
git clone https://github.com/ARM-software/ff-a-acs.git
~~~

### Porting steps

See [Arm FF-A ACS Porting Guide](./docs/porting_guide.md) document for porting steps required to run ACS for your target platform.

### Build steps

To build the ACS for your target platform, perform the following steps:<br />

```
cd ff-a-acs ;
mkdir build ; cd build
cmake ../ -G"Unix Makefiles" -DCROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- -DTARGET=<platform_name>
make
```
<br />Options information:<br />
-   -G"<generator_name>" : "Unix Makefiles" to generate Makefiles for Linux and Cygwin. "MinGW Makefiles" to generate Makefiles for cmd.exe on Windows <br />
-   -DTARGET=<platform_name> is the same as the name of the target-specific directory created in the **platform/pal_baremetal/** directory. The default vaule is -DTARGET=tgt_tfa_fvp.<br />
-   -DCROSS_COMPILE=<path_to_aarch64_gcc> Set the cross-compiler toolchain supporting AArch64 target. For example, -DCROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- <br />
-   -DCC=<path_to_armclang_or_clang_binary> To compile ACS using clang or armclang cross compiler toolchain. The default compilation is with aarch64-gcc.<br />
-   -DSUITE=<suite_name> is the sub test suite name specified in **test/** directory. The default value is -DSUITE=all
-   -DVERBOSE=<verbose_level>. Print verbosity level. Supported print levels are 1(INFO & above), 2(DEBUG & above), 3(TEST & above), 4(WARN & ERROR) and 5(ERROR). Default value is 3.
-	-DARM_ARCH_MAJOR=<major_version> The major version of Arm Architecture to target when compiling test suite. Its value must be numeric, and defaults to 8.
-	-DARM_ARCH_MINOR=<major_version> The minor version of Arm Architecture to target when compiling test suite. Its value must be numeric, and defaults to 0.
-	-DCMAKE_BUILD_TYPE=<build_type>: Chooses between a debug and release build. It can take either release or debug as values. The default value is release.
-	-DPLATFORM_SPMC_EL=<el_num>: EL number where the target SPMC component runs. Supported values are 1 and 2. The default value is 2.
-	-DPLATFORM_SP_EL=<el_num>: EL number where the test secure endpoints are expected to run. Supported values are 0 and 1. The default value is 1.
-	-DPLATFORM_NS_HYPERVISOR_PRESENT=<0|1>: Does the system support the non-secure hypervisor implementing FF-A features? 1 for yes, 0 for no. The default vaule is 1. System is expected to intergrate and load all the three of nonsecure test endpoints(vm1, vm2 and vm3) if the value is set to 1. Otherwise needs to use single non-secure test endpoint(vm1) which would act as NS OS kernel.

*To compile tests for tgt_tfa_fvp platform*:<br />
```
cd ff-a-acs ;
mkdir build ; cd build
cmake ../ -G"Unix Makefiles" -DCROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- -DTARGET=tgt_tfa_fvp -DPLATFORM_NS_HYPERVISOR_PRESENT=0
make
```
**NOTE**
	 The current release has been tested on **tgt_tfa_fvp** reference platform with build options set to -DPLATFORM_NS_HYPERVISOR_PRESENT=0, -DPLATFORM_SPMC_EL=2, -DPLATFORM_SP_EL=1. This represents system configuration where SPMD is at EL3, SPMC is at SEL2,  three test-SPs(sp1, sp2 and sp3) at SEL1 and one test NS-EP (vm1) runs at NSEL1 as an OS kernel. See [testcase_unverified](./docs/testcase_unverified.md) document to know list of unverified tests on reference platform.<br />

### Build output
The ACS build generates the binaries for following test endpoints:<br />
- build/output/sp1.bin
- build/output/sp2.bin
- build/output/sp3.bin
- build/output/vm1.bin
- build/output/vm2.bin (Generated when PLATFORM_NS_HYPERVISOR_PRESENT set to 1)
- build/output/vm3.bin (Generated when PLATFORM_NS_HYPERVISOR_PRESENT set to 1)

### Integrating the binaries into your target platform

Test endpoint manifest describes the endpoint attributes as defined in the Arm FF-A specification which would help to intergrate, load and boot the test secure and non-secure endpoints with target partition manager components.
1. Integrate test secure endpoint binaries sp1, sp2 and sp3 with your target SPMC component.
2. If non-secure hypervisor is implemented, integrate test non-secure endpoint binaries vm1, vm2 and vm3 with it. Otherwise only vm1 needs to be integrate with the secure software stack and vm1 would act as NS OS kernel.

## Test suite execution
The following steps describe the execution flow before the test execution: <br />

1. The target platform must load the above binaries into appropriate memory. <br />
2. The *System Under Test* (SUT) software boots to an environment that initializes the test secure endpoints such that sp1, sp2 and sp3 are ready to accept FF-A message requests in val_wait_for_test_fn_req function on primary boot cpu. <br />
3. On the Non-secure side, the SUT software(SPM or NS-hypervisor) boots test non-secure enpoints such that vm1 gets overall test suite control in val_test_dispatch function. And if NS-Hypervisor supported, vm2 and vm3 must be booted such that they are ready to accept FF-A message requests in val_wait_for_test_fn_req function.<br />
4. vm1 executes each test sequentially in a loop in the val_test_dispatch function.<br />

## Linux OS based tests
FF-A tests can be executed through Linux application. This needs Linux kernel module files to load Test VM1 endpoint code at kernel level. These files are hosted at [linux-acs/ffa-acs-drv](https://git.gitlab.arm.com/linux-arm/linux-acs/-/tree/master/ffa-acs-drv). The procedure to build and run FF-A tests on this configuration is described in the README file of the module package.

## Security implication
This ACS may run at higher privilege level. An attacker can utilize these tests as a means to elevate privilege which can potentially reveal the platform secure attests. To prevent such security vulnerabilities into the production system, it is strongly recommended that Arm FF-A ACS  is run on development platforms. If it is run on production system, make sure system is scrubbed after running the tests.

## License

Arm FF-A ACS is distributed under BSD-3-Clause License.


## Feedback, contributions, and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, send an email to support-ff-a-acs@arm.com with details.
 - Arm licensees can contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests.

--------------

*Copyright (c) 2021, Arm Limited or its affliates. All rights reserved.*
