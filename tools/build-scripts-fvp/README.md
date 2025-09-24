FF-A Linux User App ACS Build
=============================

This file provides instructions for building and running inside FVP the FF-A
Linux ACS prototype.

Environment setup
-----------------

This script has been tested in an Ubuntu X86 64bit Linux environment and the
following tools are required to be installed:
- git
- make
- cmake
- dtc
- mkfs.ext2
- ninja
- pip3
- wget
- curl

The script will download some required tools:
- gcc arm64 toolchain
- clang toolchain
- Fixed Virtual Platform

The script will download the required source code for:
- TF-A
- Hafnium (EL2 SPMC) or EL3 SPMC
- FF-A ACS
- Xen
- Linux kernel
- Busybox

Build the prototype
-------------------

To build the prototype, use the following command format:

   ./acs-demo.sh [--build=<target>] [--acs-dir=<path>] [--spm-level=<level>]

Examples:

Build a full system based on Hafnium:
./acs-demo.sh --build=all --spm-level=2

Build and run a full system using EL3 SPMC:
./acs-demo.sh --build=all --run --spm-level=3

Build ACS using a local ACS source tree
./acs-demo.sh --build=acs --acs-dir=/path/to/custom/acs


Run the prototype using FVP
---------------------------

To run the prototype inside FVP, use the following command format:

   ./acs-demo.sh --run --spm-level=<2|3>

Example:

   ./acs-demo.sh --run --spm-level=2


Combined build and run example
------------------------------

You can also combine build and run in a single command:

   ./acs-demo.sh --build=all --acs-dir=/path/to/custom/acs --spm-level=2 --run


Help
----

To view all available options and usage instructions, run:

   ./acs-demo.sh --help

Post Run
--------

To have an easier interaction with the Linux console, connect to it using
telnet:
  telnet localhost 5000
FVP provides the port where to connect in its early boot log, use the one
provided for console0:
  terminal_0: Listening for serial connection on port 5000

To login in the Dom0 Linux system use the user 'root', no password will be
asked.

Inside the shell, you can execute ACS using the following command:
   ~# acs-linux-app.elf

You can also create a linux guest to run ACS from it:
   ~# cd /xen-guests
   ~# xl create -c linux-guest.cfg

   Once ACS is finished you can kill the VM by doing the following:
   Go back to telnet prompt by hitting CTRL + ]
   Send an escape command:
   telnet> send escape

   This will bring you back to Dom0 console from which you can kill the VM:
    ~# xl destroy linux-guest

   From the started guest console, you can execute ACS:
   ~# acs-linux-app.elf

You can also create a baremetal ACS guest to run ACS without linux:
   ~# cd /xen-guests
   ~# xl create -c baremetal-guest.cfg

    Once ACS is done, you can kill the VM from Dom0 console:
    ~# xl destroy baremetal-guest

Distclean the prototype build
-----------------------------

To completely clean and rebuild from a fresh checkout:
   rm -rf workspace

Test Failures and Secure Partition State
----------------------------------------

During test failures, **secure partitions can enter an invalid state**, which may impact the
behavior of subsequent tests. These partitions do not automatically recover, and the system may
require a **cold reset** to restore a clean state. It is therefore **recommended to run tests
individually during bring-up**, especially when debugging or validating early setups.

You can run individual tests by editing CMD_ACS_LINUX and CMD_ACS_BAREMETAL in scripts/acs.sh by
including following Cmake variables :

-DSUITE=all -DSUITE_TEST_RANGE=”<start_test>;<end_test>”

To run a **single test**, use the same test name for both start and end. For example:

-DSUITE=all -DSUITE_TEST_RANGE=“ffa_mem_share;ffa_mem_share”

This method helps isolate failures and ensures a more stable and reproducible test environment
during development.

Prototype content
-----------------

The prototype contains the following components:
- Arm Trusted Firmware
- Hafnium
- 4 secure partitions (EL2 SPMC) or 1 secure partition (EL3 SPMC) produced by FF-A ACS and
   communicating with the ACS linux application.
- u-boot
- Xen
- Linux and a Busybox Root Filesystem started as Xen Dom0.

FFA-ACS Driver  
-----------------

Required FFA-ACS driver is built out-of-tree as a dynamically loadable kernel module within the ACS source tree:
`ff-a-acs/linux-acs/ffa-acs-drv`

FFA-ACS driver **cannot interoperate** with upstream `ffa-core` and `ffa-module` drivers — these must be unloaded before running ACS tests.

Example usage to load/unload `ffa-module` / `ffa-core` and `ffa-acs` driver:

   ~# modprobe ffa-module

   ---- ffa-module dependent apps work here ----

   ~# modprobe -r ffa-module  # unloads both ffa-module and ffa-core

   ---- ffa-module support removed ----

   ~# modprobe ffa-acs  # load ACS test driver

   ~# cd /bin

   ~# ./acs-linux-app.elf  # run ACS test binary

   ~# modprobe -r ffa-acs  # unload ACS driver

   ~# modprobe ffa-module  # reload upstream FF-A drivers

   ---- ffa-module support reestablished ----
