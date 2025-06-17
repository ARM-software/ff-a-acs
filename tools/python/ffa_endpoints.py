#!/usr/bin/env python3
#
# Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import os
import re
import glob
import sys
from datetime import datetime

# Bit shift used to encode exec_state in el_info field
EL_WIDTH_SHIFT = 4

# Maps the exception-level encoding from DTS to actual EL
EXCEPTION_LEVEL_MAP = {
    0: 1,  # EL1
    1: 0,  # S_EL0 → EL0
    2: 1   # S_EL1 → EL1
}

# Generate a license header string with the current year
def generate_license_header():
    year = datetime.now().year
    return f"""\
/*
 * Copyright (c) {year}, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
"""

# Extract a value using a regex pattern from a string (manifest content)
def extract_field(content, pattern, default=None, cast=lambda x: x):
    match = re.search(pattern, content)
    return cast(match.group(1)) if match else default

# Parse a single DTS manifest file and return a dictionary of extracted fields
def parse_manifest(path, el_mode):
    with open(path, 'r') as f:
        content = f.read()

    # Extract UUID field and convert it into a list of 32-bit integers
    uuid = [int(x, 16) for x in extract_field(content, r'uuid\s*=\s*<([^>]+)>', '').split()]
    uuid = uuid if uuid else [0, 0, 0, 0]

    # Extract ID and OR it with 0x8000 to mark as secure
    id_raw = extract_field(content, r'\bid\s*=\s*<(\d+)>', 0, int)
    id_ = id_raw | 0x8000

    # Extract other manifest properties
    tg0 = extract_field(content, r'xlat-granule\s*=\s*<(\d+)>', 0, int)
    ec_count = extract_field(content, r'execution-ctx-count\s*=\s*<(\d+)>', 1, int)
    messaging = extract_field(content, r'messaging-method\s*=\s*<(\w+)>', 0, lambda x: int(x, 16))
    notification = bool(re.search(r'notification-support', content))

    # Exception level and execution state
    exc_level_encoded = extract_field(content, r'exception-level\s*=\s*<(\d+)>', 0, int)
    exec_state = extract_field(content, r'execution-state\s*=\s*<(\d+)>', 0, int)
    exc_level = EXCEPTION_LEVEL_MAP.get(exc_level_encoded, 1)

    # Compose el_info using exec_state and exception level
    el_info = ((exec_state == 0) << EL_WIDTH_SHIFT) | exc_level

    # Calculate endpoint property bitmask
    ep = 0
    if messaging & 0x001: ep |= 1 << 0  # direct_msg
    if messaging & 0x002: ep |= 1 << 1  # indirect_msg
    if messaging & 0x004: ep |= 1 << 2  # doorbell
    if notification:      ep |= 1 << 3  # notification supported
    if exec_state == 0:   ep |= 1 << 8  # AArch64
    if messaging & 0x200: ep |= 1 << 9  # memory share
    if messaging & 0x400: ep |= 1 << 10 # memory lend

    # Extract name from filename:
    #  - Remove '_el0' or '_el1' suffix (case-insensitive)
    #  - Preserve underscores
    #  - Uppercase the name
    raw_name = os.path.splitext(os.path.basename(path))[0]
    base_name = re.sub(r'_el[01]$', '', raw_name, flags=re.IGNORECASE)
    name = base_name.upper()

    # Print parsed details for verification
    print(f"\nParsed Manifest: {os.path.basename(path)}")
    print("-" * 50)
    print(f"  name         = {name}")
    print(f"  id_raw       = {id_raw} → id = 0x{id_:04x}")
    print(f"  tg0          = {tg0}")
    print(f"  ec_count     = {ec_count}")
    print(f"  el_enc       = {exc_level_encoded} → mapped EL = {exc_level}")
    print(f"  exec_state   = {exec_state} ({'AArch64' if exec_state == 0 else 'AArch32'})")
    print(f"  el_info      = 0x{el_info:02x}")
    print(f"  messaging    = 0x{messaging:03x}")
    print(f"  notification = {'Yes' if notification else 'No'}")
    print(f"  ep_properties = 0x{ep:08x}")
    print(f"  uuid         = {', '.join(f'0x{u:08x}' for u in uuid)}")
    print("-" * 50)

    # Return all parsed fields as a dictionary
    return {
        "name": name,
        "partition_status": 0xF0, # Secure not valid
        "id": id_,
        "tg0": tg0,
        "el_info": el_info,
        "ec_count": ec_count,
        "ep_properties": ep,
        "uuid": uuid
    }

# Format a single entry into a C struct initializer string
def generate_entry(entry):
    uuid_line = ", ".join(f"0x{u:08x}" for u in entry["uuid"])
    return (
        "    {\n"
        f'        .name           = "{entry["name"]}",\n'
        f'        .partition_status = 0x{entry["partition_status"]:02x},\n'
        f'        .id             = 0x{entry["id"]:04x},\n'
        f'        .tg0            = 0x{entry["tg0"]:02x},\n'
        f'        .el_info        = 0x{entry["el_info"]:02x},\n'
        f'        .ec_count       = 0x{entry["ec_count"]:04x},\n'
        f'        .ep_properties  = 0x{entry["ep_properties"]:08x},\n'
        f'        .uuid           = {{ {uuid_line} }}\n'
        "    }"
    )

# Sort files numerically based on SP or VM index
def classify_and_sort(files):
    sp, vm = [], []
    for f in files:
        name = os.path.basename(f).lower()
        m = re.match(r'sp(\d+)', name)
        if m:
            sp.append((int(m.group(1)), f))
            continue
        m = re.match(r'vm(\d+)', name)
        if m:
            vm.append((int(m.group(1)), f))
    return [f for _, f in sorted(sp + vm)]

# Write the full C and H file containing endpoint_info array
def write_c_output(entries, output_c):
    header = generate_license_header()
    ffa_array_decl = '#include "val_endpoints.h"\n\nval_endpoint_info_t endpoint_info_table[] = {\n'
    footer = "\n};\n"

    with open(output_c, "w") as f:
        f.write(header)
        f.write("\n")
        f.write(ffa_array_decl)

        # Dummy entry at index 0
        f.write("    {\n")
        f.write('        .name           = "",\n')
        f.write('        .partition_status = 0x00,\n')
        f.write('        .id             = 0x0000,\n')
        f.write('        .tg0            = 0x00,\n')
        f.write('        .el_info        = 0x00,\n')
        f.write('        .ec_count       = 0x0000,\n')
        f.write('        .ep_properties  = 0x00000000,\n')
        f.write('        .uuid           = { 0 }\n')
        f.write("    },\n")

        f.write(",\n".join(generate_entry(e) for e in entries))
        f.write(footer)

def write_h_output(entries, output_h):
    header = generate_license_header()
    real_count = len(entries)
    total_count = real_count + 1  # includes dummy entry

    content = f"""{header}

#ifndef FFA_ENDPOINTS_H
#define FFA_ENDPOINTS_H

#include \"val.h\"
#include \"val_endpoint_info.h\"

#define FFA_ENDPOINT_START_INDEX 1
#define FFA_ENDPOINT_COUNT {total_count}

extern val_endpoint_info_t endpoint_info_table[FFA_ENDPOINT_COUNT];

#endif /* FFA_ENDPOINTS_H */
"""
    with open(output_h, "w") as f:
        f.write(content)

# Main entry point
def main(manifest_dir, el_mode_str, output_dir):
    el_mode = int(el_mode_str)
    if el_mode not in (0, 1):
        print("EL mode must be 0 (el0) or 1 (el1)")
        return

    # Recursively find all .dts files in manifest_dir
    all_files = glob.glob(os.path.join(manifest_dir, "*.dts"))

    # Filter by exception level based on suffix (_el0)
    filtered = [
        f for f in all_files if (
            f.lower().endswith('_el0.dts') if el_mode == 0 else not f.lower().endswith('_el0.dts')
        )
    ]

    ordered_files = classify_and_sort(filtered)
    entries = []
    vm1_present = False

    for path in ordered_files:
        # Parse each DTS and build entry
        entry = parse_manifest(path, el_mode)
        entries.append(entry)

        # Check if VM1 is already included
        if entry["name"] == "VM1":
            vm1_present = True

    # Ensure VM1 is included at least as a dummy
    if not vm1_present:
        print("\n[INFO] Appending dummy VM1...")
        entries.append({
            "name": "VM1",
            "partition_status": 0x0F, # Nonsecure Valid
            "id": 0x0000,
            "tg0": 0,
            "el_info": 0x11,
            "ec_count": 1,
            "ep_properties": 0x70f,
            "uuid": [0, 0, 0, 0]
        })

    output_c = os.path.join(output_dir, "val_endpoints.c")
    output_h = os.path.join(output_dir, "val_endpoints.h")
    write_c_output(entries, output_c)
    write_h_output(entries, output_h)
    print(f"\n Generated:\n  {output_c}\n  {output_h} with {len(entries)} real entries (plus 1 dummy).")

# Handle command-line execution
if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python generate_ffa_endpoints.py <manifest_dir> <el: 0|1> <output_dir>")
    else:
        main(sys.argv[1], sys.argv[2], sys.argv[3])
