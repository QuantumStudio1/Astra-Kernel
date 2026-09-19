#!/usr/bin/env python3
import sys

allowed = {"KERNEL_NAME", "SCREEN_COLUMNS", "SCREEN_ROWS", "TIMER_HZ", "MAX_TASKS", "SERIAL_DEBUG"}
values = {}
for raw in open(sys.argv[1], encoding="utf-8"):
    line = raw.strip()
    if not line or line.startswith("#"):
        continue
    key, sep, value = line.partition("=")
    if not sep or key not in allowed or not value.isdecimal():
        if key == "KERNEL_NAME" and sep and value:
            values[key] = value
            continue
        raise SystemExit(f"Invalid setting: {raw.rstrip()}")
    values[key] = value
missing = allowed - values.keys()
if missing:
    raise SystemExit("Missing settings: " + ", ".join(sorted(missing)))
with open(sys.argv[2], "w", encoding="utf-8", newline="\n") as output:
    output.write("/* Generated from config/astra.conf; do not edit. */\n#pragma once\n")
    for key in sorted(values):
        value = values[key]
        output.write(f'#define {key} "{value}"\n' if key == "KERNEL_NAME" else f"#define {key} {value}u\n")
