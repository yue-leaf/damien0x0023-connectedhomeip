/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "FactoryDataProvider.h"
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/Span.h>
#include <platform/ConfigurationManager.h>
#include <setup_payload/SetupPayload.h>
#include <string.h>

#if defined(TI_DAC_KEY_USE_PSA_HSM)
#include <psa/crypto.h>
#include <third_party/hsmddk/include/Integration/Adapter_ITS/incl/tfm_hal_its.h>

namespace {
constexpr uint32_t kTiPsaItsBase = 0x000DF000u;
constexpr size_t kTiPsaItsSize   = 0x00002000u;
constexpr uint32_t kTiNvsBase    = 0x000E1000u;

static_assert(kTiPsaItsBase + kTiPsaItsSize == kTiNvsBase, "PSA ITS must end immediately before Matter NVS");
} // namespace

extern "C" enum tfm_hal_status_t tfm_hal_its_fs_info(struct tfm_hal_its_fs_info_t * fsInfo)
{
    if (fsInfo == nullptr)
    {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    // TI's GCC fallback places ITS immediately below HSM firmware, which
    // overlaps CC27xx factory data. Match the SysConfig KeyStore region used
    // by the temporary provisioning firmware instead.
    fsInfo->flash_area_addr   = kTiPsaItsBase;
    fsInfo->flash_area_size   = kTiPsaItsSize;
    fsInfo->sectors_per_block = 1;
    return TFM_HAL_SUCCESS;
}
#endif

namespace chip {
namespace DeviceLayer {

#if defined(TI_DAC_KEY_USE_PSA_HSM)
namespace {
#ifndef TI_DAC_PSA_KEY_ID
#define TI_DAC_PSA_KEY_ID 0x00027023u
#endif
constexpr psa_key_id_t kTiDacPsaKeyId = TI_DAC_PSA_KEY_ID;

CHIP_ERROR EnsurePsaInitialized()
{
    static bool sPsaInitialized = false;

    if (sPsaInitialized)
    {
        return CHIP_NO_ERROR;
    }

    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "psa_crypto_init failed: %d", static_cast<int>(status));
        return CHIP_ERROR_INTERNAL;
    }

    sPsaInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PsaStatusToChipError(psa_status_t status)
{
    if (status != PSA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "DAC PSA signing failed: %d", static_cast<int>(status));
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR LogPsaFailure(const char * operation, psa_status_t status)
{
    if (status != PSA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "%s failed: %d", operation, static_cast<int>(status));
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

psa_status_t ComputeSha256(const uint8_t * message, size_t messageLen, uint8_t * digest, size_t digestSize)
{
    if (message == nullptr || digest == nullptr)
    {
        return PSA_ERROR_INVALID_ARGUMENT;
    }
    if (digestSize < Crypto::kSHA256_Hash_Length)
    {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    // Hash with Matter's software Crypto PAL before asking the HSM to sign the
    // digest. This keeps the attestation TBS digest identical to the
    // commissioner path, including for messages spanning many SHA-256 blocks.
    return Crypto::Hash_SHA256(message, messageLen, digest) == CHIP_NO_ERROR ? PSA_SUCCESS
                                                                            : PSA_ERROR_GENERIC_ERROR;
}

psa_status_t SignPsaDacMessage(const uint8_t * message, size_t messageLen, uint8_t * signature, size_t signatureSize,
                               size_t * signatureLen)
{
    uint8_t digest[Crypto::kSHA256_Hash_Length];
    psa_status_t status = ComputeSha256(message, messageLen, digest, sizeof(digest));
    if (status == PSA_SUCCESS)
    {
        status = psa_sign_hash(kTiDacPsaKeyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), digest, sizeof(digest), signature,
                               signatureSize, signatureLen);
    }
    memset(digest, 0, sizeof(digest));
    return status;
}
} // namespace
#endif

typedef struct
{
    uint32_t len;
    uint8_t * data;
} data_ptr;

typedef struct
{
    data_ptr serial_number;
    data_ptr vendor_id;
    data_ptr product_id;
    data_ptr vendor_name;
    data_ptr product_name;
    data_ptr manufacturing_date;
    data_ptr hw_ver;
    data_ptr hw_ver_str;
    data_ptr dac_cert;
    data_ptr dac_priv_key;
    data_ptr pai_cert;
    data_ptr rd_uniqueid;
    data_ptr spake2p_it;
    data_ptr spake2p_salt;
    data_ptr spake2p_verifier;
    data_ptr discriminator;
    data_ptr passcode;
    data_ptr certification_declaration;
} factoryData;

#if defined(__GNUC__)
factoryData __attribute__((section(".factory_data_struct"))) __attribute__((used)) mFactoryData =
#else

#error "compiler currently not supported"

#endif
    {};

namespace {
CHIP_ERROR ValidateRequiredField(const data_ptr & field, const char * name, size_t minLen, size_t maxLen)
{
    if (field.data == nullptr)
    {
        ChipLogError(DeviceLayer, "Factory Data %s is missing", name);
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if (field.len < minLen || field.len > maxLen)
    {
        ChipLogError(DeviceLayer, "Factory Data %s has invalid length: %u expected=[%u,%u]", name,
                     static_cast<unsigned>(field.len), static_cast<unsigned>(minLen), static_cast<unsigned>(maxLen));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidatePrintableStringField(const data_ptr & field, const char * name, size_t maxLen)
{
    ReturnErrorOnFailure(ValidateRequiredField(field, name, 1, maxLen));
    for (uint32_t i = 0; i < field.len; ++i)
    {
        if (field.data[i] < 0x20 || field.data[i] > 0x7e)
        {
            ChipLogError(DeviceLayer, "Factory Data %s contains non-printable byte at index %u", name,
                         static_cast<unsigned>(i));
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    return CHIP_NO_ERROR;
}

bool IsDigit(uint8_t value)
{
    return value >= '0' && value <= '9';
}

CHIP_ERROR ValidateManufacturingDate()
{
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.manufacturing_date, "manufacturing_date", 10, 10));

    const uint8_t * date = mFactoryData.manufacturing_date.data;
    if (!IsDigit(date[0]) || !IsDigit(date[1]) || !IsDigit(date[2]) || !IsDigit(date[3]) || date[4] != '-' ||
        !IsDigit(date[5]) || !IsDigit(date[6]) || date[7] != '-' || !IsDigit(date[8]) || !IsDigit(date[9]))
    {
        ChipLogError(DeviceLayer, "Factory Data manufacturing_date must be YYYY-MM-DD");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint16_t year = static_cast<uint16_t>((date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 +
                                          (date[3] - '0'));
    uint8_t month = static_cast<uint8_t>((date[5] - '0') * 10 + (date[6] - '0'));
    uint8_t day   = static_cast<uint8_t>((date[8] - '0') * 10 + (date[9] - '0'));
    if (year == 0 || month < 1 || month > 12 || day < 1 || day > 31)
    {
        ChipLogError(DeviceLayer, "Factory Data manufacturing_date is out of range: %04u-%02u-%02u",
                     static_cast<unsigned>(year), static_cast<unsigned>(month), static_cast<unsigned>(day));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadFactoryDataUint(const data_ptr & field, const char * name, size_t expectedLen, uint32_t & value)
{
    if (field.data == nullptr)
    {
        ChipLogError(DeviceLayer, "Factory Data %s is missing", name);
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if (field.len != expectedLen)
    {
        ChipLogError(DeviceLayer, "Factory Data %s has invalid length: %u", name, static_cast<unsigned>(field.len));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    value = 0;
    memcpy(&value, field.data, expectedLen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidateFactoryDataCommissionableData()
{
    uint32_t discriminator = 0;
    uint32_t passcode      = 0;
    uint32_t iterations    = 0;

    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.discriminator, "discriminator", sizeof(uint16_t), discriminator));
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.passcode, "passcode", sizeof(uint32_t), passcode));
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.spake2p_it, "spake2p_it", sizeof(uint16_t), iterations));

    if (discriminator > 4095)
    {
        ChipLogError(DeviceLayer, "Factory Data discriminator is out of range: %u", static_cast<unsigned>(discriminator));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!PayloadContents::IsValidSetupPIN(passcode))
    {
        ChipLogError(DeviceLayer, "Factory Data passcode is invalid: %u", static_cast<unsigned>(passcode));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (iterations < Crypto::kSpake2p_Min_PBKDF_Iterations || iterations > Crypto::kSpake2p_Max_PBKDF_Iterations)
    {
        ChipLogError(DeviceLayer, "Factory Data SPAKE2+ iteration count is out of range: %u",
                     static_cast<unsigned>(iterations));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (mFactoryData.spake2p_salt.data == nullptr ||
        mFactoryData.spake2p_salt.len < Crypto::kSpake2p_Min_PBKDF_Salt_Length ||
        mFactoryData.spake2p_salt.len > Crypto::kSpake2p_Max_PBKDF_Salt_Length)
    {
        ChipLogError(DeviceLayer, "Factory Data SPAKE2+ salt has invalid length: %u",
                     static_cast<unsigned>(mFactoryData.spake2p_salt.len));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (mFactoryData.spake2p_verifier.data == nullptr ||
        mFactoryData.spake2p_verifier.len != Crypto::kSpake2p_VerifierSerialized_Length)
    {
        ChipLogError(DeviceLayer, "Factory Data SPAKE2+ verifier has invalid length: %u",
                     static_cast<unsigned>(mFactoryData.spake2p_verifier.len));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ChipLogProgress(DeviceLayer,
                    "FactoryDataProvider commissioning data: passcode=%u discriminator=%u spake2_it=%u salt_len=%u verifier_len=%u",
                    static_cast<unsigned>(passcode), static_cast<unsigned>(discriminator), static_cast<unsigned>(iterations),
                    static_cast<unsigned>(mFactoryData.spake2p_salt.len),
                    static_cast<unsigned>(mFactoryData.spake2p_verifier.len));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidateFactoryDataDeviceInstance()
{
    uint32_t vendorId        = 0;
    uint32_t productId       = 0;
    uint32_t hardwareVersion = 0;

    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.vendor_id, "vendor_id", sizeof(uint16_t), vendorId));
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.product_id, "product_id", sizeof(uint16_t), productId));
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.hw_ver, "hw_ver", sizeof(uint16_t), hardwareVersion));
    ReturnErrorOnFailure(ValidatePrintableStringField(mFactoryData.serial_number, "serial_number",
                                                      ConfigurationManager::kMaxSerialNumberLength));
    ReturnErrorOnFailure(ValidatePrintableStringField(mFactoryData.vendor_name, "vendor_name",
                                                      ConfigurationManager::kMaxVendorNameLength));
    ReturnErrorOnFailure(ValidatePrintableStringField(mFactoryData.product_name, "product_name",
                                                      ConfigurationManager::kMaxProductNameLength));
    ReturnErrorOnFailure(ValidatePrintableStringField(mFactoryData.hw_ver_str, "hw_ver_str",
                                                      ConfigurationManager::kMaxHardwareVersionStringLength));
    ReturnErrorOnFailure(ValidateManufacturingDate());
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.rd_uniqueid, "rd_uniqueid", 16, 32));

    ChipLogProgress(DeviceLayer,
                    "FactoryDataProvider device instance: vid=%u pid=%u serial=%.*s vendor=%.*s product=%.*s hw_ver=%u hw_ver_str=%.*s mfg=%.*s rd_uid_len=%u",
                    static_cast<unsigned>(vendorId), static_cast<unsigned>(productId),
                    static_cast<int>(mFactoryData.serial_number.len), reinterpret_cast<const char *>(mFactoryData.serial_number.data),
                    static_cast<int>(mFactoryData.vendor_name.len), reinterpret_cast<const char *>(mFactoryData.vendor_name.data),
                    static_cast<int>(mFactoryData.product_name.len), reinterpret_cast<const char *>(mFactoryData.product_name.data),
                    static_cast<unsigned>(hardwareVersion), static_cast<int>(mFactoryData.hw_ver_str.len),
                    reinterpret_cast<const char *>(mFactoryData.hw_ver_str.data),
                    static_cast<int>(mFactoryData.manufacturing_date.len),
                    reinterpret_cast<const char *>(mFactoryData.manufacturing_date.data),
                    static_cast<unsigned>(mFactoryData.rd_uniqueid.len));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ValidateFactoryDataAttestationData()
{
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.certification_declaration, "certification_declaration", 1,
                                              Credentials::kMaxDERCertLength));
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.dac_cert, "dac_cert", 1, Credentials::kMaxDERCertLength));
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.pai_cert, "pai_cert", 1, Credentials::kMaxDERCertLength));

    Crypto::P256PublicKey dacPublicKey;
    ReturnErrorOnFailure(
        Crypto::ExtractPubkeyFromX509Cert(ByteSpan{ mFactoryData.dac_cert.data, mFactoryData.dac_cert.len }, dacPublicKey));

    Crypto::P256PublicKey paiPublicKey;
    ReturnErrorOnFailure(
        Crypto::ExtractPubkeyFromX509Cert(ByteSpan{ mFactoryData.pai_cert.data, mFactoryData.pai_cert.len }, paiPublicKey));

#if !defined(TI_DAC_KEY_USE_PSA_HSM)
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.dac_priv_key, "dac_priv_key", Crypto::kP256_PrivateKey_Length,
                                              Crypto::kP256_PrivateKey_Length));
#endif

    ChipLogProgress(DeviceLayer, "FactoryDataProvider attestation data: CD_len=%u DAC_len=%u PAI_len=%u DAC_key_source=%s",
                    static_cast<unsigned>(mFactoryData.certification_declaration.len),
                    static_cast<unsigned>(mFactoryData.dac_cert.len), static_cast<unsigned>(mFactoryData.pai_cert.len),
#if defined(TI_DAC_KEY_USE_PSA_HSM)
                    "HSM");
#else
                    "factory-data");
#endif
    return CHIP_NO_ERROR;
}
} // namespace

#if defined(TI_DAC_KEY_USE_PSA_HSM)
namespace {
CHIP_ERROR ValidatePsaDacKey()
{
    ReturnErrorOnFailure(EnsurePsaInitialized());
    ReturnErrorCodeIf(!mFactoryData.dac_cert.data || mFactoryData.dac_cert.len == 0,
                      CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    Crypto::P256PublicKey certPublicKey;
    ReturnErrorOnFailure(
        Crypto::ExtractPubkeyFromX509Cert(ByteSpan{ mFactoryData.dac_cert.data, mFactoryData.dac_cert.len }, certPublicKey));

    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    Crypto::P256PublicKey hsmPublicKey;
    size_t hsmPublicKeyLen = 0;

    status = psa_get_key_attributes(kTiDacPsaKeyId, &attributes);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }

    if (psa_get_key_type(&attributes) != PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1) ||
        psa_get_key_bits(&attributes) != Crypto::kP256_PrivateKey_Length * 8)
    {
        status = PSA_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    status = psa_export_public_key(kTiDacPsaKeyId, hsmPublicKey.Bytes(), hsmPublicKey.Length(), &hsmPublicKeyLen);
    if (status != PSA_SUCCESS)
    {
        goto exit;
    }

    if (hsmPublicKeyLen != hsmPublicKey.Length() ||
        memcmp(hsmPublicKey.ConstBytes(), certPublicKey.ConstBytes(), certPublicKey.Length()) != 0)
    {
        ChipLogError(DeviceLayer, "DAC certificate public key does not match HSM key");
        status = PSA_ERROR_INVALID_SIGNATURE;
        goto exit;
    }

    {
        static const uint8_t kValidationMessage[] = { 'T', 'I', ' ', 'D', 'A', 'C', ' ', 'H', 'S', 'M' };
        uint8_t validationDigest[Crypto::kSHA256_Hash_Length];
        Crypto::P256ECDSASignature validationSignature;
        size_t validationSignatureLen = 0;

        status =
            ComputeSha256(kValidationMessage, sizeof(kValidationMessage), validationDigest, sizeof(validationDigest));
        if (status == PSA_SUCCESS)
        {
            status =
                psa_sign_hash(kTiDacPsaKeyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), validationDigest, sizeof(validationDigest),
                              validationSignature.Bytes(), validationSignature.Capacity(), &validationSignatureLen);
        }
        if (status != PSA_SUCCESS)
        {
            memset(validationDigest, 0, sizeof(validationDigest));
            goto exit;
        }

        if (validationSignature.SetLength(validationSignatureLen) != CHIP_NO_ERROR ||
            certPublicKey.ECDSA_validate_hash_signature(validationDigest, sizeof(validationDigest),
                                                        validationSignature) != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "DAC HSM hash-signature self-test failed");
            status = PSA_ERROR_INVALID_SIGNATURE;
        }
        memset(validationDigest, 0, sizeof(validationDigest));
        if (status != PSA_SUCCESS)
        {
            goto exit;
        }
    }

exit:
    psa_reset_key_attributes(&attributes);
    return LogPsaFailure("DAC PSA key validation", status);
}
}
#endif

FactoryDataProvider & FactoryDataProvider::GetDefaultInstance()
{
    static FactoryDataProvider sInstance;
    return sInstance;
}

CHIP_ERROR LoadKeypairFromRaw(ByteSpan private_key, ByteSpan public_key, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(private_key.size() + public_key.size()));
    memcpy(serialized_keypair.Bytes(), public_key.data(), public_key.size());
    memcpy(serialized_keypair.Bytes() + public_key.size(), private_key.data(), private_key.size());
    return keypair.Deserialize(serialized_keypair);
}

CHIP_ERROR FactoryDataProvider::Init()
{
    ReturnErrorOnFailure(ValidateFactoryDataDeviceInstance());
    ReturnErrorOnFailure(ValidateFactoryDataAttestationData());
    ReturnErrorOnFailure(ValidateFactoryDataCommissionableData());
#if defined(TI_DAC_KEY_USE_PSA_HSM)
    ReturnErrorOnFailure(ValidatePsaDacKey());
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & out_buffer)
{
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.certification_declaration, "certification_declaration", 1,
                                               Credentials::kMaxDERCertLength));
    ReturnErrorCodeIf(out_buffer.size() < mFactoryData.certification_declaration.len, CHIP_ERROR_BUFFER_TOO_SMALL);

    return CopySpanToMutableSpan(ByteSpan{ mFactoryData.certification_declaration.data,
                                           mFactoryData.certification_declaration.len },
                                 out_buffer);
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & outBuffer)
{
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.dac_cert, "dac_cert", 1, Credentials::kMaxDERCertLength));
    ReturnErrorCodeIf(outBuffer.size() < mFactoryData.dac_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(outBuffer.data(), mFactoryData.dac_cert.data, mFactoryData.dac_cert.len);
    outBuffer.reduce_size(mFactoryData.dac_cert.len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.pai_cert, "pai_cert", 1, Credentials::kMaxDERCertLength));
    ReturnErrorCodeIf(outBuffer.size() < mFactoryData.pai_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(outBuffer.data(), mFactoryData.pai_cert.data, mFactoryData.pai_cert.len);
    outBuffer.reduce_size(mFactoryData.pai_cert.len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);

#if defined(TI_DAC_KEY_USE_PSA_HSM)
    ReturnErrorOnFailure(EnsurePsaInitialized());
    VerifyOrReturnError(outSignBuffer.size() >= Crypto::kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t signatureLen = 0;
    psa_status_t status = SignPsaDacMessage(messageToSign.data(), messageToSign.size(), outSignBuffer.data(),
                                            outSignBuffer.size(), &signatureLen);
    ReturnErrorOnFailure(PsaStatusToChipError(status));
    VerifyOrReturnError(signatureLen == Crypto::kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "DAC PSA signature length is %u", static_cast<unsigned>(signatureLen)));

    outSignBuffer.reduce_size(signatureLen);
    return CHIP_NO_ERROR;
#else
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Extract public key from DAC cert.
    uint8_t dacBuf[600] = { 0 };
    MutableByteSpan dacCertSpan(dacBuf);
    ReturnErrorCodeIf(dacCertSpan.size() < mFactoryData.dac_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(dacCertSpan.data(), mFactoryData.dac_cert.data, mFactoryData.dac_cert.len);
    dacCertSpan.reduce_size(mFactoryData.dac_cert.len);
    chip::Crypto::P256PublicKey dacPublicKey;

    ReturnErrorOnFailure(chip::Crypto::ExtractPubkeyFromX509Cert(dacCertSpan, dacPublicKey));

    uint8_t privKeyBuf[600] = { 0 };
    MutableByteSpan privKeySpan(privKeyBuf);
    ReturnErrorCodeIf(privKeySpan.size() < mFactoryData.dac_priv_key.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(privKeySpan.data(), mFactoryData.dac_priv_key.data, mFactoryData.dac_priv_key.len);
    privKeySpan.reduce_size(mFactoryData.dac_priv_key.len);

    uint8_t pubKeyBuf[600] = { 0 };
    MutableByteSpan pubKeySpan(pubKeyBuf);
    ReturnErrorCodeIf(pubKeySpan.size() < dacPublicKey.Length(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(pubKeySpan.data(), dacPublicKey.Bytes(), dacPublicKey.Length());
    pubKeySpan.reduce_size(dacPublicKey.Length());

    ReturnErrorOnFailure(LoadKeypairFromRaw(privKeySpan, pubKeySpan, keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
#endif
}
CHIP_ERROR FactoryDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    uint32_t value = 0;
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.discriminator, "discriminator", sizeof(uint16_t), value));
    ReturnErrorCodeIf(value > 4095, CHIP_ERROR_INVALID_ARGUMENT);
    setupDiscriminator = static_cast<uint16_t>(value);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.spake2p_it, "spake2p_it", sizeof(uint16_t), iterationCount));
    ReturnErrorCodeIf(iterationCount < Crypto::kSpake2p_Min_PBKDF_Iterations ||
                          iterationCount > Crypto::kSpake2p_Max_PBKDF_Iterations,
                      CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.spake2p_salt, "spake2p_salt",
                                               Crypto::kSpake2p_Min_PBKDF_Salt_Length,
                                               Crypto::kSpake2p_Max_PBKDF_Salt_Length));
    ReturnErrorCodeIf(saltBuf.size() < mFactoryData.spake2p_salt.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), mFactoryData.spake2p_salt.data, mFactoryData.spake2p_salt.len);
    saltBuf.reduce_size(mFactoryData.spake2p_salt.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.spake2p_verifier, "spake2p_verifier",
                                               Crypto::kSpake2p_VerifierSerialized_Length,
                                               Crypto::kSpake2p_VerifierSerialized_Length));
    ReturnErrorCodeIf(verifierBuf.size() < mFactoryData.spake2p_verifier.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), mFactoryData.spake2p_verifier.data, mFactoryData.spake2p_verifier.len);
    verifierLen = mFactoryData.spake2p_verifier.len;
    verifierBuf.reduce_size(verifierLen);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.passcode, "passcode", sizeof(uint32_t), setupPasscode));
    ReturnErrorCodeIf(!PayloadContents::IsValidSetupPIN(setupPasscode), CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    ReturnErrorOnFailure(ValidatePrintableStringField(mFactoryData.vendor_name, "vendor_name",
                                                      ConfigurationManager::kMaxVendorNameLength));
    ReturnErrorCodeIf(bufSize < mFactoryData.vendor_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf, mFactoryData.vendor_name.data, mFactoryData.vendor_name.len);
    buf[mFactoryData.vendor_name.len] = '\0';
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    uint32_t value = 0;
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.vendor_id, "vendor_id", sizeof(uint16_t), value));
    vendorId = static_cast<uint16_t>(value);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    ReturnErrorOnFailure(ValidatePrintableStringField(mFactoryData.product_name, "product_name",
                                                      ConfigurationManager::kMaxProductNameLength));
    ReturnErrorCodeIf(bufSize < mFactoryData.product_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    memset(buf, 0, bufSize);
    memcpy(buf, mFactoryData.product_name.data, mFactoryData.product_name.len);
    buf[mFactoryData.product_name.len] = '\0';
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    uint32_t value = 0;
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.product_id, "product_id", sizeof(uint16_t), value));
    productId = static_cast<uint16_t>(value);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetPartNumber(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetProductURL(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetProductLabel(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR FactoryDataProvider::GetSerialNumber(char * buf, size_t bufSize)
{
    ReturnErrorOnFailure(ValidatePrintableStringField(mFactoryData.serial_number, "serial_number",
                                                      ConfigurationManager::kMaxSerialNumberLength));
    ReturnErrorCodeIf(bufSize < mFactoryData.serial_number.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    memset(buf, 0, bufSize);
    memcpy(buf, mFactoryData.serial_number.data, mFactoryData.serial_number.len);
    buf[mFactoryData.serial_number.len] = '\0';
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    ReturnErrorOnFailure(ValidateManufacturingDate());
    uint8_t tmp[10] = { 0 };
    memcpy(tmp, mFactoryData.manufacturing_date.data, 10);
    year  = ((tmp[0] - 0x30) * 1000) + ((tmp[1] - 0x30) * 100) + ((tmp[2] - 0x30) * 10) + (tmp[3] - 0x30);
    month = ((tmp[5] - 0x30) * 10) + (tmp[6] - 0x30);
    day   = ((tmp[8] - 0x30) * 10) + (tmp[9] - 0x30);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    uint32_t value = 0;
    ReturnErrorOnFailure(ReadFactoryDataUint(mFactoryData.hw_ver, "hw_ver", sizeof(uint16_t), value));
    hardwareVersion = static_cast<uint16_t>(value);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    ReturnErrorOnFailure(ValidatePrintableStringField(mFactoryData.hw_ver_str, "hw_ver_str",
                                                      ConfigurationManager::kMaxHardwareVersionStringLength));
    ReturnErrorCodeIf(bufSize < mFactoryData.hw_ver_str.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    memset(buf, 0, bufSize);
    memcpy(buf, mFactoryData.hw_ver_str.data, mFactoryData.hw_ver_str.len);
    buf[mFactoryData.hw_ver_str.len] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ReturnErrorOnFailure(ValidateRequiredField(mFactoryData.rd_uniqueid, "rd_uniqueid", 16, 32));
    ReturnErrorCodeIf(uniqueIdSpan.size() < mFactoryData.rd_uniqueid.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(uniqueIdSpan.data(), mFactoryData.rd_uniqueid.data, mFactoryData.rd_uniqueid.len);
    uniqueIdSpan.reduce_size(mFactoryData.rd_uniqueid.len);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
