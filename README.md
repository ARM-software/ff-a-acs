
# Architecture Compliance Suite for Arm Firmware Framework for Armv8-A

## Arm Firmware Framework for Armv8-A

*Arm Firmware Framework for Armv8-A (FF-A)* describes a software architecture that achieves the following goals:

1. Applies the Virtualization Extension to isolate software images provided by different vendors.
2. Describes the interfaces that standardize communication between the various software images. This includes communication between images in the Secure world and Normal world.

Arm FF-A also goes beyond the above stated goals to ensure that the interfaces can be used to standardize communication:
- In the absence of the Virtualization Extensions in the Secure world. This provides a migration path for existing Secure world software images to a system that implements the Virtualization Extension in the Secure state.
- Between Virtual Machines (VMs) managed by a hypervisor in the Normal world. The Virtualization Extensions in the Secure state mirrors their counterparts in the Non-secure state. The hypervisor uses the firmware framework interfaces to enable communication between VMs that it manages.

The following are the main components of Arm FF-A:
- A Partition Manager (PM), which manages partitions is the hypervisor in Normal world and the Secure Partition Manager (SPM) in Secure world.
- One or more partitions that are sandboxes created by the PM could be VMs in Normal world or Secure world. The VMs in Secure world are called Secure Partitions (SP).
- Application Binary Interfaces (ABIs) that partitions can invoke to communicate with other partitions.
- A partition manifest that describes the system resources a partition needs, services that a partition implements, and other attributes of the partition that governs its runtime behavior.

**Note** : In this document, the terms Endpoint (EP) and partition are used interchangeably.

For more information, download the [Arm FF-A Specification](https://developer.arm.com/docs/den0077/latest)

### Architecture Compliance Suite

The Architecture Compliance Suite (ACS) contains a set of functional tests, demonstrating the invariant behaviors that are specified in the architecture specification. It is used to ensure architecture compliance of the implementations to Arm FF-A specification. The example implementations are SPMD(SPM Dispatcher) and SPMC(SPM Core) components in Secure world and hyperviosr in Normal World.

This suite contains self-checking, and portable C and assembly based tests with directed stimulus. These tests are available as open source. The tests and the corresponding abstraction layers are available with a BSD-3-Clause License allowing for external contribution.

This suite is not a substitute for design verification. To review the test logs, you can contact Arm directly through your partner managers.

For more information on Architecture Compliance Suite see [Validation Methodology](./docs/Arm_FF_A_ACS_Validation_Methodology.pdf) document.

## This release
- Release Version - v0.8
- Code Quality: Beta - ACS is being developed, please use this opportunity to ameliorate.
- The tests are written for Arm FF-A 1.1 specification version.
- For information about the test coverage scenarios that are implemented in the current release of ACS and the scenarios that are planned for the future releases, see [Testcase checklist](./docs/testcase_checklist.md).

## GitHub branch
- To pick up the release version of the code, checkout the release branch.
- To get the latest version of the code with bug fixes and new features, use the main branch.

## Software requirements

The following tools are required to build the ACS: <br />
- Host operating system: Ubuntu 18.04, RHEL 7
- Git to download ACS repository
- CMake 3.17
- Cross-compiler toolchain supporting AArch64 target: GCC >= 9.2-2019.12 can be downloaded from [Arm Developer website](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads)

**Note** : ACS can also be compiled using Arm Compiler 6 (Verified with 6.12) or Clang (Verified with 8.0.0) Cross-compiler toolchain. To compile and assemble sources using these toolchains, set -DCC=<compiler-path> with path pointing to the armclang or clang binary. For linking, ACS needs the GNU linker for generating the test binaries regardless of chosen toolchain.

## Download source

To download the main branch of the repository, type the following command: <br />
~~~
git clone https://github.com/ARM-software/ff-a-acs.git
~~~

### Porting steps

For more information on porting steps to run ACS for the target platform, see [Arm FF-A ACS Porting Guide](./docs/porting_guide.md) document.

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
-	-DPLATFORM_SP_EL=<el_num>: EL number where the test secure endpoints are expected to run. Supported values are 0(EL0), 1(EL1), and -1(Platform doesn't support deploying FFA based SPs). The default value is 1.
-	-DPLATFORM_NS_HYPERVISOR_PRESENT=<0|1>: Does the system support the non-secure hypervisor implementing FF-A features? 1 for yes, 0 for no. The default vaule is 1. System is expected to intergrate and load all the three of nonsecure test endpoints(vm1, vm2 and vm3) if the value is set to 1. Otherwise needs to use single non-secure test endpoint(vm1) which would act as NS OS kernel.
-   -DPLATFORM_FFA_V_1_0=<0|1>: It runs only tests that are supported by the Arm FF-A v1.0 specification. The default value is 0.
-   -DPLATFORM_FFA_V_1_1=<0|1>: It only tests the Arm FF-A v1.1 specifications as updates to Arm FF-A v1.0. The default value is 0.
-   -DPLATFORM_FFA_V_ALL=<0|1>: It runs all tests that are supported by the Arm FF-A v1.1 specification. The default value is 1.

*To compile tests for tgt_tfa_fvp platform*:<br />
```
cd ff-a-acs ;
mkdir build ; cd build
cmake ../ -G"Unix Makefiles" -DCROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf- -DTARGET=tgt_tfa_fvp -DPLATFORM_NS_HYPERVISOR_PRESENT=0
make
```
**NOTE**
	 The current release has been tested on **tgt_tfa_fvp** reference platforms with build options set to -DPLATFORM_NS_HYPERVISOR_PRESENT=0, -DPLATFORM_SPMC_EL=2, -DPLATFORM_SP_EL=1. These platform represents system configuration where SPMD and SMPC are implemented at EL3 and SEL2 respectively, and three test-SPs(sp1, sp2 and sp3) runs at SEL1 and one test NS-EP (vm1) runs as an OS kernel in normal world. For more information on the unverified tests on reference platform, see [testcase_unverified](./docs/testcase_unverified.md) document.<br />

### Build output
The ACS build generates the binaries for the following test endpoints:<br />
- build/output/sp1.bin
- build/output/sp2.bin
- build/output/sp3.bin
- build/output/vm1.bin
- build/output/vm2.bin (Generated when PLATFORM_NS_HYPERVISOR_PRESENT set to 1)
- build/output/vm3.bin (Generated when PLATFORM_NS_HYPERVISOR_PRESENT set to 1)

For information on integrating the binaries into the target platform, test suite execution flow, analysing the test results and more, see [Validation Methodology](./docs/Arm_FF_A_ACS_Validation_Methodology.pdf) document.

## Security implication
The ACS tests may run at higher privilege level. An attacker can utilize these tests to elevate privilege which can potentially reveal the platform secure attests. To prevent such security vulnerabilities into the production system, it is recommended that FF-A ACS is run on development platforms. If it is run on production system, ensure that the system is scrubbed after running the tests.

## License

Arm FF-A ACS is distributed under BSD-3-Clause License.


## Feedback, contributions, and support

 - For feedback, use the GitHub Issue Tracker that is associated with this repository.
 - For support, send an email to support-ff-a-acs@arm.com with details.
 - Arm licensees can contact Arm directly through their partner managers.
 - Arm welcomes code contributions through GitHub pull requests.

--------------

*Copyright (c) 2021-2022, Arm Limited or its affiliates. All rights reserved.*
