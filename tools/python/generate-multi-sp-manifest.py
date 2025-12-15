#!/usr/bin/env python3
#
# Copyright (c) 2026, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import argparse
import json
import sys
import os

SP_NAMES = ["sp1", "sp2", "sp3", "sp4"]

EL_LEVEL = ["0", "1"]

VERSION_MAP = {
    "11": "v11",
    "12": "v12",
}


def compute_pm(version, sp_name, el):
    """Compute DTS path based on version and EL rules."""
    if version not in VERSION_MAP:
        print(f"ERROR: Version {version} currently not supported")
        sys.exit(1)

    folder = "../../../platform/manifest/tgt_tfa_fvp/" + VERSION_MAP[version]

    # EL rule:
    # EL0 -> use spX_el0.dts
    # EL1 -> use spX.dts
    if el == "0":
        dts = f"{sp_name}_el0.dts"
    else:
        dts = f"{sp_name}.dts"

    return f"{folder}/{dts}"


def main():
    parser = argparse.ArgumentParser(description="Generate SP manifest")

    parser.add_argument("--el", required=True,
                        help="EL level: 0 => spX_el0.dts, 1 => spX.dts")

    parser.add_argument("--sp", nargs="+", required=True,
                        help="List of 4 SP versions (1.1 or 1.2)")

    args = parser.parse_args()

    versions = args.sp
    el = args.el

    # Must have exactly 4 versions
    if len(versions) != 4:
        print("ERROR: Please give all 4 values for SP")
        sys.exit(1)

    # Validate supported versions
    for v in versions:
        if v not in VERSION_MAP:
            print(f"ERROR: Version {v} currently not supported")
            sys.exit(1)

    if el not in EL_LEVEL:
            print(f"ERROR: Invalid EL Level {el}")
            sys.exit(1)

    manifest = {}

    # Build manifest entries
    for sp_name, version in zip(SP_NAMES, versions):
        pm_path = compute_pm(version, sp_name, el)
        image_path = f"../../../build/output/{sp_name}.bin"

        manifest[sp_name] = {"image": image_path, "pm": pm_path}

    # choose path under BUILD dir
    path = "build/generated/manifest"
    os.makedirs(path, exist_ok=True)

    outfile_el0 = os.path.join(path, "sp_layout_el0_multi.json")
    outfile     = os.path.join(path, "sp_layout_multi.json")

    if el == "0":
        with open(outfile_el0, "w") as f:
            json.dump(manifest, f, indent=4)

        print("Generated sp_layout_el0_multi.json successfully!")
    else:
        with open(outfile, "w") as f:
            json.dump(manifest, f, indent=4)

        print("Generated sp_layout_multi.json successfully!")



if __name__ == "__main__":
    main()

