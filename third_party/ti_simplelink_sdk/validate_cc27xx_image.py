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

"""Validate a CC27xx Matter image before publishing it for provisioning."""

import argparse
import re
from pathlib import Path
from typing import Tuple

from intelhex import IntelHex


FACTORY_DATA_MAX_SIZE = 0x1000
EXPECTED_CHIP_ERASE_RETAIN = 0xFF800000
EXPECTED_PSA_KEY_CONTEXT_SIZE = 0x1C0
EXPECTED_PSA_ARCHIVE = "/third_party/psa_crypto/lib/ticlang/m33f/psa_crypto_cc27xx.a"

DEVICE_LAYOUTS = {
    "cc2745r10": {
        "factory_data_address": 0x000E7000,
        "nvs_address": 0x000E1000,
        "protected_flash_start": 0x000DC000,
        "protected_flash_end": 0x00100000,
        "ccfg_chip_erase_retain_address": 0x4E02003C,
    },
    "cc2755p20": {
        "factory_data_address": 0x001E7000,
        "nvs_address": 0x001E1000,
        "protected_flash_start": 0x001DC000,
        "protected_flash_end": 0x00200000,
        "ccfg_chip_erase_retain_address": 0x4E02004C,
    },
}


def parse_factory_section(map_path: Path) -> Tuple[int, int]:
    pattern = re.compile(r"^\s*\.factory_data\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)")
    with map_path.open(encoding="utf-8", errors="replace") as map_file:
        for line in map_file:
            match = pattern.match(line)
            if match:
                return tuple(int(value, 16) for value in match.groups())
    raise ValueError("Missing .factory_data section in Matter map file")


def parse_nvs_section(map_path: Path) -> Tuple[int, int]:
    pattern = re.compile(r"^\s*\.nvs\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)")
    with map_path.open(encoding="utf-8", errors="replace") as map_file:
        for line in map_file:
            match = pattern.match(line)
            if match:
                return tuple(int(value, 16) for value in match.groups())
    raise ValueError("Missing .nvs section in Matter map file")


def parse_psa_key_context_size(map_path: Path) -> int:
    pattern = re.compile(r"^\s*\.bss\.gl_PSA_Key\s*$")
    size_pattern = re.compile(r"^\s*0x[0-9a-fA-F]+\s+0x([0-9a-fA-F]+)\s+")
    with map_path.open(encoding="utf-8", errors="replace") as map_file:
        lines = iter(map_file)
        for line in lines:
            if not pattern.match(line):
                continue
            details = next(lines, "")
            match = size_pattern.match(details)
            if match:
                return int(match.group(1), 16)
            break
    raise ValueError("Missing gl_PSA_Key allocation in Matter map file")


def validate_psa_archive(map_path: Path) -> None:
    map_text = map_path.read_text(encoding="utf-8", errors="replace").replace("\\", "/")
    if EXPECTED_PSA_ARCHIVE not in map_text:
        raise ValueError("Matter application must link the shared TI Clang PSA archive")


def validate_image(map_path: Path, hex_path: Path, device: str) -> None:
    layout = DEVICE_LAYOUTS[device]
    factory_address, factory_size = parse_factory_section(map_path)
    if factory_address != layout["factory_data_address"] or not 0 < factory_size <= FACTORY_DATA_MAX_SIZE:
        raise ValueError(
            f"Unsafe .factory_data section: address={factory_address:#x}, size={factory_size:#x}"
        )

    nvs_address, nvs_size = parse_nvs_section(map_path)
    if nvs_address != layout["nvs_address"] or nvs_size != 0x6000:
        raise ValueError(f"Unsafe .nvs section: address={nvs_address:#x}, size={nvs_size:#x}")

    psa_key_context_size = parse_psa_key_context_size(map_path)
    if psa_key_context_size != EXPECTED_PSA_KEY_CONTEXT_SIZE:
        raise ValueError(
            "Matter PSA KeyStore slot layout does not match the provisioning firmware: "
            f"gl_PSA_Key size={psa_key_context_size:#x}, expected={EXPECTED_PSA_KEY_CONTEXT_SIZE:#x}"
        )
    validate_psa_archive(map_path)

    image = IntelHex(str(hex_path))
    addresses = image.addresses()
    if not addresses:
        raise ValueError("Matter application HEX is empty")

    protected = [
        address
        for address in addresses
        if layout["protected_flash_start"] <= address < layout["protected_flash_end"]
    ]
    if protected:
        raise ValueError(
            "Matter application HEX contains PSA/NVS/factory/HSM data: "
            f"{min(protected):#x}-{max(protected):#x}"
        )

    retain_bytes = bytes(image[address] for address in range(
        layout["ccfg_chip_erase_retain_address"],
        layout["ccfg_chip_erase_retain_address"] + 4,
    ))
    retain_value = int.from_bytes(retain_bytes, "little")
    if retain_value != EXPECTED_CHIP_ERASE_RETAIN:
        raise ValueError(
            "Matter CCFG does not retain the PSA/NVS/factory/HSM flash block: "
            f"chipEraseRetain={retain_value:#010x}, "
            f"expected={EXPECTED_CHIP_ERASE_RETAIN:#010x}"
        )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--device", choices=DEVICE_LAYOUTS, default="cc2745r10")
    parser.add_argument("--map", required=True, type=Path, dest="map_path")
    parser.add_argument("--hex", required=True, type=Path, dest="hex_path")
    args = parser.parse_args()

    try:
        validate_image(args.map_path, args.hex_path, args.device)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    print("Matter HEX protected-range validation passed")


if __name__ == "__main__":
    main()
