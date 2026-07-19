#!/usr/bin/env python3

# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Validate the CC2745 Matter image before publishing it for provisioning."""

import argparse
import re
from pathlib import Path
from typing import Tuple

from intelhex import IntelHex


FACTORY_DATA_ADDRESS = 0x000E7000
FACTORY_DATA_MAX_SIZE = 0x1000
PROTECTED_FLASH_START = 0x000DF000
PROTECTED_FLASH_END = 0x00100000


def parse_factory_section(map_path: Path) -> Tuple[int, int]:
    pattern = re.compile(r"^\s*\.factory_data\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)")
    with map_path.open(encoding="utf-8", errors="replace") as map_file:
        for line in map_file:
            match = pattern.match(line)
            if match:
                return tuple(int(value, 16) for value in match.groups())
    raise ValueError("Missing .factory_data section in Matter map file")


def validate_image(map_path: Path, hex_path: Path) -> None:
    factory_address, factory_size = parse_factory_section(map_path)
    if factory_address != FACTORY_DATA_ADDRESS or not 0 < factory_size <= FACTORY_DATA_MAX_SIZE:
        raise ValueError(
            f"Unsafe .factory_data section: address={factory_address:#x}, size={factory_size:#x}"
        )

    addresses = IntelHex(str(hex_path)).addresses()
    if not addresses:
        raise ValueError("Matter application HEX is empty")

    protected = [address for address in addresses if PROTECTED_FLASH_START <= address < PROTECTED_FLASH_END]
    if protected:
        raise ValueError(
            "Matter application HEX contains PSA/NVS/factory/HSM data: "
            f"{min(protected):#x}-{max(protected):#x}"
        )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--map", required=True, type=Path, dest="map_path")
    parser.add_argument("--hex", required=True, type=Path, dest="hex_path")
    args = parser.parse_args()

    try:
        validate_image(args.map_path, args.hex_path)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    print("Matter HEX protected-range validation passed")


if __name__ == "__main__":
    main()
