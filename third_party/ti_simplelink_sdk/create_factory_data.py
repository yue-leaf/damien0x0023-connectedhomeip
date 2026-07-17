#!/usr/bin/env python

# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import json
import subprocess
import sys
import re

import intelhex
from jsonschema import validate

FACTORY_DATA_FIELD_ORDER = [
    "serial_number",
    "vendor_id",
    "product_id",
    "vendor_name",
    "product_name",
    "manufacturing_date",
    "hw_ver",
    "hw_ver_str",
    "dac_cert",
    "dac_priv_key",
    "pai_cert",
    "rd_uniqueid",
    "spake2_it",
    "spake2_salt",
    "spake2_verifier",
    "discriminator",
    "passcode",
    "certification_declaration",
]

FACTORY_DATA_FIELD_ALIASES = {
    "rd_uid": "rd_uniqueid",
}

REQUIRED_FACTORY_DATA_FIELDS = [
    key for key in FACTORY_DATA_FIELD_ORDER if key != "dac_priv_key"
]


def _element_key(element):
    keys = [key for key in element.keys() if key != "len"]
    if len(keys) != 1:
        raise ValueError("Each factory data element must contain exactly one value key and one len key")
    return FACTORY_DATA_FIELD_ALIASES.get(keys[0], keys[0])


def _normalize_element(key, element):
    if key in element:
        return element

    raw_key = next(raw_key for raw_key in element.keys() if raw_key != "len")
    return {key: element[raw_key], "len": element["len"]}


def _empty_element(key):
    return {key: "hex:", "len": 0}


def _element_bytes(key, element):
    len_integer = element["len"]
    value = element[key]

    if isinstance(value, str):
        value = value.strip(" ")
        if value.startswith("hex:"):
            hex_value = value[4:]
            if len(hex_value) % 2 != 0:
                raise ValueError(f"Factory data element {key} has an odd number of hex characters")
            data = bytes.fromhex(hex_value)
        else:
            data = value.encode("ascii")
    else:
        data = int(value).to_bytes(len_integer, "little")

    if len(data) != len_integer:
        raise ValueError(f"Factory data element {key} length is {len_integer}, but value has {len(data)} bytes")
    return data


def create_hex_file(args):
    # create empty factory data file
    factory_data_intelhex = intelhex.IntelHex()
    factory_data_struct_intelhex = intelhex.IntelHex()

    device_family = args.device_family
    dac_key_storage = getattr(args, "dac_key_storage", None)
    if dac_key_storage is None:
        if device_family == "cc27xx":
            raise ValueError("cc27xx factory data generation requires explicit --dac_key_storage hsm or factory_data")
        dac_key_storage = "factory_data"
    matter_app_map_file = args.matter_app_map_file
    factory_data_base_address = 0
    factory_data_length = None

    # Each element in the struct uses 8 bytes: 4 for the element length and
    # 4 for the pointer to the element data.
    factory_data_dict = json.load(args.factory_data_json_file[0])
    factory_data_schema = json.load(args.factory_data_schema[0])

    validate(factory_data_dict, factory_data_schema)
    factory_data_elements = {}
    for element in factory_data_dict['elements']:
        key = _element_key(element)
        if key in factory_data_elements:
            raise ValueError(f"Factory data element {key} is duplicated")
        factory_data_elements[key] = _normalize_element(key, element)

    unknown_keys = set(factory_data_elements.keys()) - set(FACTORY_DATA_FIELD_ORDER)
    if unknown_keys:
        raise ValueError(f"Unknown factory data elements: {', '.join(sorted(unknown_keys))}")

    for key in REQUIRED_FACTORY_DATA_FIELDS:
        element = factory_data_elements.get(key)
        if element is None or len(_element_bytes(key, element)) == 0:
            raise ValueError(f"Factory data element {key} is required and must not be empty")

    if dac_key_storage == "hsm":
        factory_data_elements["dac_priv_key"] = _empty_element("dac_priv_key")
    elif len(_element_bytes("dac_priv_key", factory_data_elements.get("dac_priv_key", _empty_element("dac_priv_key")))) == 0:
        raise ValueError("Factory data element dac_priv_key is required when DAC signing does not use HSM")

    factory_data = [factory_data_elements.get(key, _empty_element(key)) for key in FACTORY_DATA_FIELD_ORDER]

    struct_idx = 0
    values_idx = 0

    # Retrieve Factory Data base address and length from Map file
    with open(matter_app_map_file, "r") as map_file:
        pattern = r".*\.factory_data.*(0x.*)\s*(0x.*)"

        for line in map_file:
            factoryDataResult = re.search(pattern, line)
            if factoryDataResult:
                factory_data_base_address = int(factoryDataResult.group(1), 16)
                factory_data_length = int(factoryDataResult.group(2), 16)
                break

    if factory_data_length is None:
        raise ValueError("Could not find .factory_data section in Matter application map file")

    value_address = factory_data_base_address + factory_data_length

    for element in factory_data:
        # get the length in hex and write to first hex file
        len_integer = element['len']

        factory_data_struct_intelhex[struct_idx + 3] = (len_integer & 0xFF000000) >> 24
        factory_data_struct_intelhex[struct_idx + 2] = (len_integer & 0x00FF0000) >> 16
        factory_data_struct_intelhex[struct_idx + 1] = (len_integer & 0x0000FF00) >> 8
        factory_data_struct_intelhex[struct_idx] = (len_integer & 0x000000FF)

        struct_idx += 4

        # write the address to the file and increment by the size of the element
        factory_data_struct_intelhex[struct_idx + 3] = (value_address & 0xFF000000) >> 24
        factory_data_struct_intelhex[struct_idx + 2] = (value_address & 0x00FF0000) >> 16
        factory_data_struct_intelhex[struct_idx + 1] = (value_address & 0x0000FF00) >> 8
        factory_data_struct_intelhex[struct_idx] = (value_address & 0x000000FF)

        struct_idx += 4
        value_address += len_integer

        key = _element_key(element)
        for value_byte in _element_bytes(key, element):
            factory_data_intelhex[values_idx] = value_byte
            values_idx += 1

    if struct_idx > factory_data_length:
        raise ValueError("Factory data JSON contains more elements than the firmware factoryData struct")

    # Pad any optional struct entries that were not present in the JSON. This
    # keeps element data from overwriting trailing struct fields.
    while struct_idx < factory_data_length:
        factory_data_struct_intelhex[struct_idx] = 0
        struct_idx += 1

    # merge both hex files
    idx = 0
    while idx < values_idx:
        factory_data_struct_intelhex[struct_idx] = factory_data_intelhex[idx]
        idx = idx + 1
        struct_idx = struct_idx + 1

    # output to hex file
    factory_data_struct_intelhex.tofile(args.factory_data_hex_file, format='hex')
    factorydata_change_address = '--change-addresses=' + f'{factory_data_base_address:#x}'

    # get hex file in a format that can be merged in a later step
    subprocess.call(['objcopy', args.factory_data_hex_file, '--input-target', 'ihex', '--output-target', 'binary', 'temp.bin'])
    subprocess.call(['objcopy', 'temp.bin', '--input-target', 'binary', '--output-target',
                     'ihex', args.factory_data_hex_file, factorydata_change_address])
    subprocess.call(['rm', 'temp.bin'])


def main():
    parser = argparse.ArgumentParser(description="TI Factory Data hex file creator")

    parser.add_argument('-factory_data', '--factory_data_json_file', required=True,  nargs=1,
                        help="JSON file of factory data", type=argparse.FileType('r'))
    parser.add_argument('-schema', '--factory_data_schema', required=True, nargs=1,
                        help="Factory Data Schema", type=argparse.FileType('r'))
    parser.add_argument('-o', '--factory_data_hex_file', required=True)
    parser.add_argument('-m', '--matter_app_map_file', required=True)
    parser.add_argument('-device', '--device_family', required=True)
    parser.add_argument('--dac_key_storage', choices=['factory_data', 'hsm'],
                        help="Where the DAC private key is stored. Use 'hsm' to omit dac_priv_key from factory data.")

    args = parser.parse_args()
    create_hex_file(args)


if __name__ == "__main__":
    main()
