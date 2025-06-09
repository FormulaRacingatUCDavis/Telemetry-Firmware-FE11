#!/usr/bin/python3
"""
biscuit_trimmed: Utilities for parsing binary data from the car into an array
"""

import sys
from pathlib import Path

HEADER_SIZE = 8
MAGIC_NUMBER = 0xB155CC17
ENTRY_SIZE = 4 + 8 + 4


def validate_header(header):
    magic = int.from_bytes(header[0:4], "little")
    if magic != MAGIC_NUMBER:
        return False

    header_version = int.from_bytes(header[4:6], "little")
    print(f"Biscuit version for file: {header_version}")

    # TODO: Check padding?

    return True


def parse_biscuit(bin_string):
    """
    Parses a binary file and emits an array containing the relevant data.
    data_file: the file containing the binary data

    Returns an array of data rows, each containing the can message data
    """
    # Grab header first
    header = bin_string[0:HEADER_SIZE]
    if len(header) < HEADER_SIZE:
        return ["Error: invalid header"]
    if not validate_header(header):
        return ["Error: invalid header"]

    bin_string = bin_string[HEADER_SIZE-1:]

    # Enter the parse loop
    out = []
    while True:
        entry = bin_string[0:ENTRY_SIZE]
        bin_string = bin_string[ENTRY_SIZE-1:]
        if len(entry) == 0:
            break
        if len(entry) < ENTRY_SIZE:
            # TODO: emit warning?
            pass
        else:
            entry_id = int.from_bytes(entry[0:4], "little")
            # entry_id_out = f'{entry_id:x}'.upper()
            entry_tick = int.from_bytes(entry[12:16], "little")
            out.append(
                [entry_id, entry[4], entry[5], entry[6], entry[7], entry[8], entry[9], entry[10], entry[11],
                 entry_tick]);

    return out
