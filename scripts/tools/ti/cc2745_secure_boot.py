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

"""Prepare and finalize an externally signed CC2745 ROM Secure Boot image."""

from __future__ import annotations

import argparse
import base64
import hashlib
import json
import shutil
import struct
import subprocess
import sys
import types
import zlib
from dataclasses import dataclass
from pathlib import Path

from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from intelhex import IntelHex


CCFG_BASE = 0x4E020000
CCFG_SIZE = 0x800
SCFG_BASE = 0x4E040000
SCFG_SIZE = 0x400
APP_SLOT_START = 0x00000000

CCFG_BOOT_PBLDR_VTOR_OFFSET = 0x00
CCFG_BOOT_APP_VTOR_OFFSET = 0x08
CCFG_BOOT_CRC_END = 0x0C
CCFG_BOOT_CRC_OFFSET = 0x0C
CCFG_PERMISSIONS_OFFSET = 0x18
CCFG_MISC_OFFSET = 0x1C
CCFG_WRITE_ERASE_PROTECTION_OFFSET = 0x20
CCFG_CHIP_ERASE_RETAIN_OFFSET = 0x3C
CCFG_MAIN_CRC_BEGIN = 0x10
CCFG_MAIN_CRC_END = 0x74C
CCFG_MAIN_CRC_OFFSET = 0x74C
CCFG_DEBUG_CRC_BEGIN = 0x7D0
CCFG_DEBUG_CRC_END = 0x7FC
CCFG_DEBUG_CRC_OFFSET = 0x7FC

SCFG_FLASH_LAYOUT_OFFSET = 0x78
SCFG_POLICY_OFFSET = 0xA4
SCFG_KEY_UPDATE_HASH_OFFSET = 0xB0
SCFG_PERMISSIONS_OFFSET = 0xD0
SCFG_CRC_OFFSET = 0xE4
SCFG_KEY_RING_OFFSET = 0xE8
SCFG_KEY_ENTRY_SIZE = 44
SCFG_KEY_ENTRY_COUNT = 18

SCFG_POLICY_SIGNATURE = 0x8B635A5A
SCFG_POLICY_ALG_ECDSA_P256_SHA256 = 0x03B95A5A
SCFG_POLICY_OVERWRITE = 0x92B1A5A5
SCFG_KEY_ENTRY_TYPE_APP = 0x3F68A5A5
SCFG_KEY_ENTRY_STATUS_ACTIVE = 0x59183D3B
SCFG_UNDEFINED_SLOT = 0xFFFFFFFF
CCFG_PRIMARY_BOOTLOADER_FORBID = 0xFFFFFFFC
PERMISSION_ALLOW = 0xA
EXPECTED_CHIP_ERASE_RETAIN = 0xFF800000

PROTECTED_FLASH_START = 0x000DC000
PROTECTED_FLASH_END = 0x00100000

UNSIGNED_BIN_NAME = "unsigned-ti-cc27xx-lighting.bin"
XCFG_HEX_NAME = "xcfg.hex"
DIGEST_NAME = "ti-cc27xx-imgtool-to-sign.sha256"
SIGNATURE_DER_NAME = "ti-cc27xx-signature.der"
SIGNATURE_B64_NAME = "ti-cc27xx-signature.b64"
PUBLIC_KEY_NAME = "public_key.pem"
SIGNED_BIN_NAME = "signed-ti-cc27xx-lighting.bin"
SIGNED_BEFORE_CONFIG_NAME = "signed-ti-cc27xx-lighting-before-secure-boot-config.hex"
ORIGINAL_XCFG_NAME = "matter-original-xcfg-from-signed.hex"
SECURITY_CONFIG_NAME = "security-config.hex"
FINAL_SIGNED_HEX_NAME = "signed-ti-cc27xx-lighting-without-factory-data.hex"
PREPARE_MANIFEST_NAME = "prepare-manifest.json"
MANIFEST_NAME = "manifest.json"


class SecureBootError(ValueError):
    pass


@dataclass(frozen=True)
class SecureBootProperties:
    header_size: int
    slot_size: int
    version: str
    image_type: int
    security_counter: int
    rom_fixed: int


@dataclass(frozen=True)
class ExtractedImage:
    data: bytes
    base_address: int
    xcfg_memory: dict[int, int]


def _parse_int(value: object, name: str) -> int:
    try:
        return int(str(value), 0)
    except (TypeError, ValueError) as error:
        raise SecureBootError(f"invalid {name}: {value!r}") from error


def load_properties(path: Path) -> SecureBootProperties:
    raw = json.loads(path.read_text(encoding="utf-8"))
    properties = SecureBootProperties(
        header_size=_parse_int(raw["header_size"], "header_size"),
        slot_size=_parse_int(raw["slot_size"], "slot_size"),
        version=str(raw["version"]),
        image_type=_parse_int(raw["image_type"], "image_type"),
        security_counter=_parse_int(raw["security_counter"], "security_counter"),
        rom_fixed=_parse_int(raw["rom_fixed"], "rom_fixed"),
    )
    if properties.header_size != properties.rom_fixed:
        raise SecureBootError("header_size and rom_fixed must match for the CC2745 image layout")
    if properties.slot_size != PROTECTED_FLASH_START:
        raise SecureBootError("slot_size must end at the CC2745 retained flash boundary 0xDC000")
    return properties


def _run_checked(name: str, command: list[str]) -> None:
    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    if completed.returncode:
        raise SecureBootError(
            f"{name} failed with exit code {completed.returncode}\n"
            f"stdout: {completed.stdout.strip()}\n"
            f"stderr: {completed.stderr.strip()}"
        )


def _resolve_executable(value: str) -> str:
    path = Path(value)
    if path.exists():
        return str(path.resolve())
    resolved = shutil.which(value)
    if resolved is None:
        raise SecureBootError(f"executable not found: {value}")
    return resolved


def _load_imgtool(scripts_dir: Path):
    if not (scripts_dir / "imgtool.py").is_file() or not (scripts_dir / "imgtool").is_dir():
        raise SecureBootError(f"TI imgtool scripts not found: {scripts_dir}")
    scripts_text = str(scripts_dir.resolve())
    if scripts_text not in sys.path:
        sys.path.insert(0, scripts_text)

    try:
        import cbor2  # noqa: F401
    except ImportError:
        cbor2_stub = types.ModuleType("cbor2")

        def unsupported_cbor(*_args, **_kwargs):
            raise SecureBootError("cbor2 is required when imgtool boot records are enabled")

        cbor2_stub.dumps = unsupported_cbor
        sys.modules["cbor2"] = cbor2_stub

    from imgtool import image as ti_image
    from imgtool import keys as ti_keys
    from imgtool import main as ti_main

    # TI SDK 9.20 omits this public-key class from the external-signature lookup.
    # Patch the imported table in memory; the installed TI SDK remains untouched.
    ti_image.ALLOWED_KEY_SHA.setdefault(ti_keys.ECDSA256P1Public, ["256"])
    return ti_main


def _run_imgtool(ti_main, args: list[str]) -> None:
    try:
        ti_main.imgtool.main(args=args, prog_name="imgtool.py", standalone_mode=False)
    except Exception as error:
        raise SecureBootError(f"TI imgtool failed: {' '.join(args)}\n{error}") from error


def _imgtool_sign_args(properties: SecureBootProperties) -> list[str]:
    return [
        "sign",
        "--header-size",
        str(properties.header_size),
        "--align",
        "4",
        "--pad-header",
        "--overwrite-only",
        "--slot-size",
        str(properties.slot_size),
        "--version",
        properties.version,
        "--rom-fixed",
        hex(properties.rom_fixed),
        "--security-counter",
        str(properties.security_counter),
        "--custom-tlv",
        "0xA0",
        f"0x{properties.image_type:08X}",
    ]


def _read_hex(path: Path) -> dict[int, int]:
    image = IntelHex(str(path))
    return {address: image[address] for address in image.addresses()}


def _write_hex(path: Path, memory: dict[int, int]) -> None:
    image = IntelHex()
    for address, value in sorted(memory.items()):
        image[address] = value
    image.write_hex_file(str(path), byte_count=16)


def _read_region(memory: dict[int, int], base: int, size: int, name: str) -> bytes:
    missing = [address for address in range(base, base + size) if address not in memory]
    if missing:
        raise SecureBootError(f"firmware does not contain complete {name}; missing 0x{missing[0]:08X}")
    return bytes(memory[address] for address in range(base, base + size))


def _replace_region(memory: dict[int, int], base: int, data: bytes) -> None:
    for offset, value in enumerate(data):
        memory[base + offset] = value


def _extract_image(
    input_path: Path,
    reference_hex: Path,
    output_dir: Path,
    objcopy: str,
    properties: SecureBootProperties,
) -> ExtractedImage:
    app_bin = output_dir / UNSIGNED_BIN_NAME
    app_hex = output_dir / ".objcopy-app.hex"
    xcfg_hex = output_dir / XCFG_HEX_NAME
    remove_config = [
        "--remove-section=.factory_data",
        "--remove-section=.ccfg",
        "--remove-section=.scfg",
    ]

    _run_checked(
        "arm-none-eabi-objcopy",
        [objcopy, "-O", "binary", "--gap-fill=0xff", *remove_config, str(input_path), str(app_bin)],
    )
    _run_checked(
        "arm-none-eabi-objcopy",
        [objcopy, "-O", "ihex", *remove_config, str(input_path), str(app_hex)],
    )
    _run_checked(
        "arm-none-eabi-objcopy",
        [
            objcopy,
            "-O",
            "ihex",
            "--only-section=.ccfg",
            "--only-section=.scfg",
            str(input_path),
            str(xcfg_hex),
        ],
    )

    app_memory = _read_hex(app_hex)
    app_hex.unlink(missing_ok=True)
    if not app_memory:
        raise SecureBootError("objcopy did not extract application data from the OUT file")
    app_base = min(app_memory)
    app_end = max(app_memory) + 1
    app_data = bytes(app_memory.get(address, 0xFF) for address in range(app_base, app_end))
    if app_bin.read_bytes() != app_data:
        raise SecureBootError("objcopy BIN and Intel HEX application payloads do not match")
    if app_base != properties.rom_fixed:
        raise SecureBootError(f"application starts at 0x{app_base:08X}; expected ROM load address 0x{properties.rom_fixed:08X}")

    xcfg_memory = _read_hex(xcfg_hex)
    allowed_xcfg = set(range(CCFG_BASE, CCFG_BASE + CCFG_SIZE)) | set(range(SCFG_BASE, SCFG_BASE + SCFG_SIZE))
    unexpected_xcfg = sorted(set(xcfg_memory) - allowed_xcfg)
    if unexpected_xcfg:
        raise SecureBootError(f"xcfg.hex contains unexpected address 0x{unexpected_xcfg[0]:08X}")
    _read_region(xcfg_memory, CCFG_BASE, CCFG_SIZE, "CCFG")
    _read_region(xcfg_memory, SCFG_BASE, SCFG_SIZE, "SCFG")
    _assert_erase_and_program_enabled(xcfg_memory)

    ccfg = _read_region(xcfg_memory, CCFG_BASE, CCFG_SIZE, "CCFG")
    app_vtor = _u32(ccfg, CCFG_BOOT_APP_VTOR_OFFSET)
    if app_vtor != properties.rom_fixed:
        raise SecureBootError(f"CCFG pAppVtor is 0x{app_vtor:08X}; expected 0x{properties.rom_fixed:08X}")

    reference_memory = _read_hex(reference_hex)
    reference_data = bytes(reference_memory.get(address, 0xFF) for address in range(app_base, app_end))
    if reference_data != app_data:
        raise SecureBootError("OUT application payload does not match the reference without-factory-data HEX")
    for address in allowed_xcfg:
        if reference_memory.get(address) != xcfg_memory[address]:
            raise SecureBootError(f"OUT and reference HEX configuration differ at 0x{address:08X}")
    protected = [address for address in reference_memory if PROTECTED_FLASH_START <= address < PROTECTED_FLASH_END]
    if protected:
        raise SecureBootError(
            f"reference HEX contains retained PSA/NVS/factory/HSM data at 0x{min(protected):08X}-0x{max(protected):08X}"
        )
    return ExtractedImage(data=app_data, base_address=app_base, xcfg_memory=xcfg_memory)


def prepare(args: argparse.Namespace) -> None:
    input_path = args.input.resolve()
    reference_hex = args.reference_hex.resolve()
    output_dir = args.output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    properties = load_properties(args.properties.resolve())
    objcopy = _resolve_executable(args.objcopy)
    extracted = _extract_image(input_path, reference_hex, output_dir, objcopy, properties)
    ti_main = _load_imgtool(args.imgtool_dir.resolve())

    unsigned_bin = output_dir / UNSIGNED_BIN_NAME
    digest_path = output_dir / DIGEST_NAME
    _run_imgtool(
        ti_main,
        _imgtool_sign_args(properties) + ["--vector-to-sign", "digest", str(unsigned_bin), str(digest_path)],
    )
    digest = digest_path.read_bytes()
    if len(digest) != hashlib.sha256().digest_size:
        raise SecureBootError(f"imgtool digest is {len(digest)} bytes; expected 32")

    manifest = {
        "phase": "prepared-for-external-signing",
        "source_out": input_path.name,
        "reference_hex": reference_hex.name,
        "application_base": f"0x{extracted.base_address:08X}",
        "application_size": len(extracted.data),
        "properties": _properties_json(properties),
        "sha256": {
            "source_out": _file_sha256(input_path),
            "reference_hex": _file_sha256(reference_hex),
            "unsigned_bin": _file_sha256(unsigned_bin),
            "xcfg_hex": _file_sha256(output_dir / XCFG_HEX_NAME),
            "imgtool_digest": digest.hex(),
        },
    }
    _write_json(output_dir / PREPARE_MANIFEST_NAME, manifest)
    print(f"Prepared TI imgtool digest: {digest_path}")


def _load_p256_public_key(path: Path) -> tuple[bytes, bytes]:
    pem = path.read_bytes()
    try:
        key = serialization.load_pem_public_key(pem)
    except ValueError as error:
        raise SecureBootError("SNB public key is not valid PEM") from error
    if not isinstance(key, ec.EllipticCurvePublicKey) or not isinstance(key.curve, ec.SECP256R1):
        raise SecureBootError("SNB public key must use ECDSA P-256 (secp256r1)")
    canonical_pem = key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo,
    )
    der = key.public_bytes(
        encoding=serialization.Encoding.DER,
        format=serialization.PublicFormat.SubjectPublicKeyInfo,
    )
    return canonical_pem, der


def _configure_secure_boot(memory: dict[int, int], app_public_key_der: bytes, properties: SecureBootProperties) -> dict[int, int]:
    configured = dict(memory)
    ccfg = bytearray(_read_region(memory, CCFG_BASE, CCFG_SIZE, "CCFG"))
    scfg = bytearray(_read_region(memory, SCFG_BASE, SCFG_SIZE, "SCFG"))
    _force_erase_and_program_permissions(ccfg, scfg)
    struct.pack_into("<I", ccfg, CCFG_BOOT_PBLDR_VTOR_OFFSET, CCFG_PRIMARY_BOOTLOADER_FORBID)

    slots = [(APP_SLOT_START, properties.slot_size)] + [(SCFG_UNDEFINED_SLOT, SCFG_UNDEFINED_SLOT)] * 4
    for index, (address, length) in enumerate(slots):
        struct.pack_into("<II", scfg, SCFG_FLASH_LAYOUT_OFFSET + index * 8, address, length)
    struct.pack_into(
        "<III",
        scfg,
        SCFG_POLICY_OFFSET,
        SCFG_POLICY_SIGNATURE,
        SCFG_POLICY_ALG_ECDSA_P256_SHA256,
        SCFG_POLICY_OVERWRITE,
    )

    public_key_hash = hashlib.sha256(app_public_key_der).digest()
    scfg[SCFG_KEY_UPDATE_HASH_OFFSET : SCFG_KEY_UPDATE_HASH_OFFSET + 32] = public_key_hash
    for index in range(SCFG_KEY_ENTRY_COUNT):
        offset = SCFG_KEY_RING_OFFSET + index * SCFG_KEY_ENTRY_SIZE
        scfg[offset : offset + 32] = b"\xff" * 32
        scfg[offset + 32 : offset + SCFG_KEY_ENTRY_SIZE] = b"\x00" * 12
    entry = SCFG_KEY_RING_OFFSET
    scfg[entry : entry + 32] = public_key_hash
    struct.pack_into("<I", scfg, entry + 32, SCFG_KEY_ENTRY_TYPE_APP)
    struct.pack_into("<I", scfg, entry + 36, zlib.crc32(scfg[entry : entry + 36]) & 0xFFFFFFFF)
    struct.pack_into("<I", scfg, entry + 40, SCFG_KEY_ENTRY_STATUS_ACTIVE)

    _recalculate_xcfg_crcs(ccfg, scfg)
    _replace_region(configured, CCFG_BASE, ccfg)
    _replace_region(configured, SCFG_BASE, scfg)
    _validate_secure_boot_configuration(configured, public_key_hash, properties)
    return configured


def _force_erase_and_program_permissions(ccfg: bytearray, scfg: bytearray) -> None:
    permissions = _set_nibble(_u32(ccfg, CCFG_PERMISSIONS_OFFSET), 3, PERMISSION_ALLOW)
    permissions = _set_nibble(permissions, 4, PERMISSION_ALLOW)
    struct.pack_into("<I", ccfg, CCFG_PERMISSIONS_OFFSET, permissions)
    struct.pack_into(
        "<I",
        ccfg,
        CCFG_MISC_OFFSET,
        _set_nibble(_u32(ccfg, CCFG_MISC_OFFSET), 1, PERMISSION_ALLOW),
    )
    struct.pack_into("<III", ccfg, CCFG_WRITE_ERASE_PROTECTION_OFFSET, *(0xFFFFFFFF,) * 3)
    struct.pack_into("<I", ccfg, CCFG_CHIP_ERASE_RETAIN_OFFSET, EXPECTED_CHIP_ERASE_RETAIN)

    permissions = _set_nibble(_u32(scfg, SCFG_PERMISSIONS_OFFSET), 3, PERMISSION_ALLOW)
    permissions = _set_nibble(permissions, 4, PERMISSION_ALLOW)
    struct.pack_into("<I", scfg, SCFG_PERMISSIONS_OFFSET, permissions)
    struct.pack_into(
        "<I",
        scfg,
        SCFG_PERMISSIONS_OFFSET + 4,
        _set_nibble(_u32(scfg, SCFG_PERMISSIONS_OFFSET + 4), 0, PERMISSION_ALLOW),
    )


def _assert_erase_and_program_enabled(memory: dict[int, int]) -> None:
    ccfg = _read_region(memory, CCFG_BASE, CCFG_SIZE, "CCFG")
    scfg = _read_region(memory, SCFG_BASE, SCFG_SIZE, "SCFG")
    checks = [
        ("CCFG allowChipErase", _nibble(_u32(ccfg, CCFG_PERMISSIONS_OFFSET), 3)),
        ("CCFG allowFlashProgram", _nibble(_u32(ccfg, CCFG_PERMISSIONS_OFFSET), 4)),
        ("CCFG allowMainAppErase", _nibble(_u32(ccfg, CCFG_MISC_OFFSET), 1)),
        ("SCFG allowChipErase", _nibble(_u32(scfg, SCFG_PERMISSIONS_OFFSET), 3)),
        ("SCFG allowFlashProgram", _nibble(_u32(scfg, SCFG_PERMISSIONS_OFFSET), 4)),
        ("SCFG allowMainAppErase", _nibble(_u32(scfg, SCFG_PERMISSIONS_OFFSET + 4), 0)),
    ]
    for name, value in checks:
        if value != PERMISSION_ALLOW:
            raise SecureBootError(f"{name}=0x{value:X}; expected ALLOW (0xA)")
    if struct.unpack_from("<III", ccfg, CCFG_WRITE_ERASE_PROTECTION_OFFSET) != (
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
    ):
        raise SecureBootError("CCFG write/erase protection locks main flash sectors")
    retain = _u32(ccfg, CCFG_CHIP_ERASE_RETAIN_OFFSET)
    if retain != EXPECTED_CHIP_ERASE_RETAIN:
        raise SecureBootError(f"CCFG chip erase retain is 0x{retain:08X}; expected 0x{EXPECTED_CHIP_ERASE_RETAIN:08X}")


def _validate_secure_boot_configuration(memory: dict[int, int], public_key_hash: bytes, properties: SecureBootProperties) -> None:
    _assert_erase_and_program_enabled(memory)
    ccfg = _read_region(memory, CCFG_BASE, CCFG_SIZE, "CCFG")
    scfg = _read_region(memory, SCFG_BASE, SCFG_SIZE, "SCFG")
    if _u32(ccfg, CCFG_BOOT_PBLDR_VTOR_OFFSET) != CCFG_PRIMARY_BOOTLOADER_FORBID:
        raise SecureBootError("CCFG primary bootloader is not forbidden")
    if _u32(ccfg, CCFG_BOOT_APP_VTOR_OFFSET) != properties.rom_fixed:
        raise SecureBootError("CCFG application VTOR does not match the signed image")
    if _u32(ccfg, CCFG_BOOT_CRC_OFFSET) != zlib.crc32(ccfg[:CCFG_BOOT_CRC_END]) & 0xFFFFFFFF:
        raise SecureBootError("CCFG boot CRC is invalid")
    if _u32(ccfg, CCFG_MAIN_CRC_OFFSET) != zlib.crc32(ccfg[CCFG_MAIN_CRC_BEGIN:CCFG_MAIN_CRC_END]) & 0xFFFFFFFF:
        raise SecureBootError("CCFG main CRC is invalid")
    if _u32(ccfg, CCFG_DEBUG_CRC_OFFSET) != zlib.crc32(ccfg[CCFG_DEBUG_CRC_BEGIN:CCFG_DEBUG_CRC_END]) & 0xFFFFFFFF:
        raise SecureBootError("CCFG debug CRC is invalid")
    if _u32(scfg, SCFG_CRC_OFFSET) != zlib.crc32(scfg[:SCFG_CRC_OFFSET]) & 0xFFFFFFFF:
        raise SecureBootError("SCFG CRC is invalid")
    if struct.unpack_from("<III", scfg, SCFG_POLICY_OFFSET) != (
        SCFG_POLICY_SIGNATURE,
        SCFG_POLICY_ALG_ECDSA_P256_SHA256,
        SCFG_POLICY_OVERWRITE,
    ):
        raise SecureBootError("SCFG policy is not Signature/ECDSA-P256/Overwrite")
    if struct.unpack_from("<II", scfg, SCFG_FLASH_LAYOUT_OFFSET) != (
        APP_SLOT_START,
        properties.slot_size,
    ):
        raise SecureBootError("SCFG primary application slot is invalid")
    if scfg[SCFG_KEY_UPDATE_HASH_OFFSET : SCFG_KEY_UPDATE_HASH_OFFSET + 32] != public_key_hash:
        raise SecureBootError("SCFG KUI public-key hash does not match the SNB public key")
    entry = scfg[SCFG_KEY_RING_OFFSET : SCFG_KEY_RING_OFFSET + SCFG_KEY_ENTRY_SIZE]
    if entry[:32] != public_key_hash or _u32(entry, 32) != SCFG_KEY_ENTRY_TYPE_APP:
        raise SecureBootError("SCFG application key-ring entry does not match the SNB public key")
    if _u32(entry, 36) != zlib.crc32(entry[:36]) & 0xFFFFFFFF:
        raise SecureBootError("SCFG application key-ring CRC is invalid")
    if _u32(entry, 40) != SCFG_KEY_ENTRY_STATUS_ACTIVE:
        raise SecureBootError("SCFG application key-ring entry is not active")


def _recalculate_xcfg_crcs(ccfg: bytearray, scfg: bytearray) -> None:
    struct.pack_into("<I", scfg, SCFG_CRC_OFFSET, zlib.crc32(scfg[:SCFG_CRC_OFFSET]) & 0xFFFFFFFF)
    struct.pack_into("<I", ccfg, CCFG_BOOT_CRC_OFFSET, zlib.crc32(ccfg[:CCFG_BOOT_CRC_END]) & 0xFFFFFFFF)
    struct.pack_into(
        "<I",
        ccfg,
        CCFG_MAIN_CRC_OFFSET,
        zlib.crc32(ccfg[CCFG_MAIN_CRC_BEGIN:CCFG_MAIN_CRC_END]) & 0xFFFFFFFF,
    )
    struct.pack_into(
        "<I",
        ccfg,
        CCFG_DEBUG_CRC_OFFSET,
        zlib.crc32(ccfg[CCFG_DEBUG_CRC_BEGIN:CCFG_DEBUG_CRC_END]) & 0xFFFFFFFF,
    )


def finalize(args: argparse.Namespace) -> None:
    output_dir = args.output_dir.resolve()
    properties = load_properties(args.properties.resolve())
    unsigned_bin = output_dir / UNSIGNED_BIN_NAME
    xcfg_hex = output_dir / XCFG_HEX_NAME
    digest_path = output_dir / DIGEST_NAME
    for path in (unsigned_bin, xcfg_hex, digest_path, args.signature, args.public_key):
        if not path.is_file():
            raise SecureBootError(f"required signing input does not exist: {path}")

    canonical_pem, public_key_der = _load_p256_public_key(args.public_key.resolve())
    public_key_path = output_dir / PUBLIC_KEY_NAME
    public_key_path.write_bytes(canonical_pem)
    digest = digest_path.read_bytes()
    if len(digest) != hashlib.sha256().digest_size:
        raise SecureBootError("imgtool digest must be 32 bytes")
    signature_der = args.signature.resolve().read_bytes()
    if not signature_der:
        raise SecureBootError("SNB signature output is empty")

    signature_path = output_dir / SIGNATURE_DER_NAME
    if args.signature.resolve() != signature_path:
        signature_path.write_bytes(signature_der)
    signature_b64 = output_dir / SIGNATURE_B64_NAME
    signature_b64.write_bytes(base64.b64encode(signature_der) + b"\n")

    ti_main = _load_imgtool(args.imgtool_dir.resolve())
    signed_bin = output_dir / SIGNED_BIN_NAME
    _run_imgtool(
        ti_main,
        _imgtool_sign_args(properties)
        + [
            "--public-key-format",
            "full",
            "--fix-sig",
            str(signature_b64),
            "--fix-sig-pubkey",
            str(public_key_path),
            str(unsigned_bin),
            str(signed_bin),
        ],
    )
    if signed_bin.stat().st_size > properties.slot_size:
        raise SecureBootError(f"signed image size 0x{signed_bin.stat().st_size:X} exceeds slot 0x{properties.slot_size:X}")
    xcfg_memory = _read_hex(xcfg_hex)
    signed_data = signed_bin.read_bytes()
    signed_memory = {APP_SLOT_START + offset: value for offset, value in enumerate(signed_data)}
    signed_memory.update(xcfg_memory)
    _write_hex(output_dir / SIGNED_BEFORE_CONFIG_NAME, signed_memory)
    _write_hex(output_dir / ORIGINAL_XCFG_NAME, xcfg_memory)

    configured = _configure_secure_boot(signed_memory, public_key_der, properties)
    signed_slot_before = {
        address: value for address, value in signed_memory.items() if APP_SLOT_START <= address < properties.slot_size
    }
    signed_slot_after = {
        address: value for address, value in configured.items() if APP_SLOT_START <= address < properties.slot_size
    }
    if signed_slot_before != signed_slot_after:
        raise SecureBootError("patching CCFG/SCFG changed the signed application slot")
    if any(PROTECTED_FLASH_START <= address < PROTECTED_FLASH_END for address in configured):
        raise SecureBootError("final signed HEX contains retained PSA/NVS/factory/HSM data")

    security_config = {
        address: value
        for address, value in configured.items()
        if CCFG_BASE <= address < CCFG_BASE + CCFG_SIZE or SCFG_BASE <= address < SCFG_BASE + SCFG_SIZE
    }
    security_config_path = output_dir / SECURITY_CONFIG_NAME
    final_hex = output_dir / FINAL_SIGNED_HEX_NAME
    _write_hex(security_config_path, security_config)
    _write_hex(final_hex, configured)

    prepare_manifest = json.loads((output_dir / PREPARE_MANIFEST_NAME).read_text(encoding="utf-8"))
    manifest = {
        "phase": "externally-signed",
        "source": prepare_manifest,
        "secure_boot": "Signature/ECDSA-P256/SHA-256/Overwrite",
        "key_policy": "same-external-SNB-key-for-application-and-KUI",
        "private_key_exported": False,
        "signed_app_unchanged_while_patching_xcfg": True,
        "imgtool_digest_sha256_hex": digest.hex(),
        "application_public_key_sha256": hashlib.sha256(public_key_der).hexdigest(),
        "key_update_public_key_sha256": hashlib.sha256(public_key_der).hexdigest(),
        "properties": _properties_json(properties),
        "sha256": {
            "signature_der": _file_sha256(signature_path),
            "public_key_pem": _file_sha256(public_key_path),
            "signed_bin": _file_sha256(signed_bin),
            "security_config_hex": _file_sha256(security_config_path),
            "final_signed_hex": _file_sha256(final_hex),
        },
    }
    _write_json(output_dir / MANIFEST_NAME, manifest)
    print(f"Externally signed CC2745 image: {final_hex}")


def _properties_json(properties: SecureBootProperties) -> dict[str, object]:
    return {
        "header_size": f"0x{properties.header_size:X}",
        "slot_size": f"0x{properties.slot_size:X}",
        "version": properties.version,
        "image_type": f"0x{properties.image_type:08X}",
        "security_counter": properties.security_counter,
        "rom_fixed": f"0x{properties.rom_fixed:X}",
    }


def _file_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _write_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def _u32(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def _nibble(value: int, index: int) -> int:
    return (value >> (index * 4)) & 0xF


def _set_nibble(value: int, index: int, nibble: int) -> int:
    shift = index * 4
    return (value & ~(0xF << shift)) | (nibble << shift)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    prepare_parser = subparsers.add_parser("prepare", help="extract the image and create the KMS digest")
    prepare_parser.add_argument("--input", type=Path, required=True)
    prepare_parser.add_argument("--reference-hex", type=Path, required=True)
    prepare_parser.add_argument("--objcopy", default="arm-none-eabi-objcopy")
    prepare_parser.add_argument("--imgtool-dir", type=Path, required=True)
    prepare_parser.add_argument("--properties", type=Path, required=True)
    prepare_parser.add_argument("--output-dir", type=Path, required=True)
    prepare_parser.set_defaults(handler=prepare)

    finalize_parser = subparsers.add_parser("finalize", help="verify the SNB signature and build final images")
    finalize_parser.add_argument("--signature", type=Path, required=True)
    finalize_parser.add_argument("--public-key", type=Path, required=True)
    finalize_parser.add_argument("--imgtool-dir", type=Path, required=True)
    finalize_parser.add_argument("--properties", type=Path, required=True)
    finalize_parser.add_argument("--output-dir", type=Path, required=True)
    finalize_parser.set_defaults(handler=finalize)
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        args.handler(args)
    except (OSError, KeyError, json.JSONDecodeError, SecureBootError) as error:
        parser.error(str(error))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
