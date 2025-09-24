#!/bin/bash
#-------------------------------------------------------------------------------
# Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set -e  # Exit on error

# --------------------------------------------------------------------------------------------------
# Argument Parsing using GNU getopt
OPTIONS=b:rh
LONGOPTS=build:,run,spm-level:,acs-dir:,help

# -temporarily store output to be evaluated
PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@")

if [[ $? -ne 0 ]]; then
    echo "Error: Failed to parse command-line arguments"
    exit 2
fi

# -Apply the parsed args
eval set -- "$PARSED"

# --------------------------------------------------------------------------------------------------
# Variables to track state
BUILD=false
RUN=false
BUILD_TARGET=""
SPM_EL_LEVEL=""
ACS_DIR_OVERRIDE=""

usage() {
    echo "Usage: $0 [--build=<target>] [--run|-r] [--spm-level=<level>] [--acs-dir=<path>] [--help|-h]"
    echo
    echo "Options:"
    echo "  -b, --build=<target>        Build the specified target (e.g., all)"
    echo "  -r, --run                   Run the test environment (no target needed)"
    echo "      --spm-level=<level>     Specify the SPM level (2 or 3)"
    echo "      --acs-dir=<path>        Specify the path to the ACS directory"
    echo "  -h, --help                  Show this help message"
}

# --------------------------------------------------------------------------------------------------
# Parse options
while true; do
    case "$1" in
        -b|--build)
            case "${2#=}" in
                linux|dts|u-boot|xen|acs|hafnium|tfa|busybox-guest|buildroot-xen|ffa-acs-driver)
                    BUILD_TARGET="${2#=}"
                    ;;
                all)
                    BUILD_TARGET="dts u-boot xen acs hafnium tfa linux busybox-guest buildroot-xen ffa-acs-driver"
                    ;;
                *)
                    echo "Error: Unknown build target '${2#=}'"
                    usage
                    exit 1
                    ;;
            esac
            BUILD=true
            shift 2
            ;;
        -r|--run)
            RUN=true
            shift
            ;;
        --spm-level)
            SPM_EL_LEVEL="${2#=}"
            shift 2
            ;;
        --acs-dir)
            ACS_DIR_OVERRIDE="${2#=}"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# --------------------------------------------------------------------------------------------------
# Validate args
if [ "$BUILD" = false ] && [ "$RUN" = false ]; then
    echo "Error: You must specify at least --build or --run"
    usage
    exit 1
fi

if [[ -n "$SPM_EL_LEVEL" ]]; then
    if [[ "$SPM_EL_LEVEL" != "2" && "$SPM_EL_LEVEL" != "3" ]]; then
        echo "Error: Invalid --spm-level. Must be 2 or 3."
        exit 1
    fi
else
    if [[ "$BUILD" == true ]]; then
        SPM_EL_LEVEL="2"
        echo "SPM_EL_LEVEL not specified. Defaulting to 2."
    else
        echo "Error: Specify --spm-level for run"
        exit 1
    fi
fi

if [[ -n "$ACS_DIR_OVERRIDE" ]]; then
    if [[ ! -d "$ACS_DIR_OVERRIDE" ]]; then
        echo "Error: Provided --acs-dir '$ACS_DIR_OVERRIDE' does not exist or is not a directory."
        exit 1
    fi
    echo "Using ACS directory: $ACS_DIR_OVERRIDE"
fi

# --------------------------------------------------------------------------------------------------
# Environment setup
ROOT_DIR=$PWD
WORKSPACE="$PWD/workspace"
SCRIPTS="$PWD/scripts"
CONFIGS="$PWD/configs"
PRECOMPILED="$PWD/precompiled"

mkdir -p "$WORKSPACE/output"
rsync -a "$SCRIPTS" "$WORKSPACE/"
rsync -a "$CONFIGS" "$WORKSPACE/"

check_dependency() {
    local path="$1"
    local var_name="$2"
    if [ ! -e "$path" ]; then
        echo "Error: Dependency '$var_name' not found at $path. Exiting."
        exit 1
    fi
}

check_dependency "$CONFIGS" "CONFIGS"
check_dependency "$SCRIPTS" "SCRIPTS"

source "$ROOT_DIR/scripts/toolchain.sh"

build() {
    echo "Building $1 with SPM_EL_LEVEL=$SPM_EL_LEVEL..."
    source "$PWD/scripts/$1.sh"

    if [[ "$1" == "u-boot" || "$1" == "hafnium" || "$1" == "acs" ]]; then
        echo "Warning: Recompilation of TFA is required after building $1."
    fi
}

run() {
    echo "Running with SPM_EL_LEVEL=$SPM_EL_LEVEL..."
    source "$PWD/scripts/run.sh"
}

# --------------------------------------------------------------------------------------------------
# Execution
if $BUILD; then
    if [[ -z "$BUILD_TARGET" ]]; then
        echo "Error: Missing build target."
        usage
        exit 1
    fi
    for comp in $BUILD_TARGET; do
        build "$comp"
    done
fi

if $RUN; then
    run
fi

echo "Operation completed successfully!"
