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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/Span.h>
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
#if defined(TI_DAC_KEY_USE_PSA_HSM)
    return ValidatePsaDacKey();
#else
    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & out_buffer)
{
    ReturnErrorCodeIf(!mFactoryData.certification_declaration.data || mFactoryData.certification_declaration.len == 0,
                      CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
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
    ReturnErrorCodeIf(outBuffer.size() < mFactoryData.dac_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.dac_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(outBuffer.data(), mFactoryData.dac_cert.data, mFactoryData.dac_cert.len);
    outBuffer.reduce_size(mFactoryData.dac_cert.len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer)
{
    ReturnErrorCodeIf(outBuffer.size() < mFactoryData.pai_cert.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.pai_cert.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
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
    ReturnErrorCodeIf(sizeof(setupDiscriminator) < mFactoryData.discriminator.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.discriminator.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&setupDiscriminator, mFactoryData.discriminator.data, mFactoryData.discriminator.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::SetSetupDiscriminator(uint16_t setupDiscriminator)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    ReturnErrorCodeIf(sizeof(iterationCount) < mFactoryData.spake2p_it.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.spake2p_it.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memset(&iterationCount, 0, sizeof(iterationCount));
    memcpy(&iterationCount, mFactoryData.spake2p_it.data, mFactoryData.spake2p_it.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetSpake2pSalt(MutableByteSpan & saltBuf)
{
    ReturnErrorCodeIf(saltBuf.size() < mFactoryData.spake2p_salt.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.spake2p_salt.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(saltBuf.data(), mFactoryData.spake2p_salt.data, mFactoryData.spake2p_salt.len);
    saltBuf.reduce_size(mFactoryData.spake2p_salt.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen)
{
    ReturnErrorCodeIf(verifierBuf.size() < mFactoryData.spake2p_verifier.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.spake2p_verifier.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(verifierBuf.data(), mFactoryData.spake2p_verifier.data, mFactoryData.spake2p_verifier.len);
    verifierLen = mFactoryData.spake2p_verifier.len;
    verifierBuf.reduce_size(verifierLen);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    ReturnErrorCodeIf(sizeof(setupPasscode) < mFactoryData.passcode.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.passcode.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&setupPasscode, mFactoryData.passcode.data, mFactoryData.passcode.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::SetSetupPasscode(uint32_t setupPasscode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR FactoryDataProvider::GetVendorName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < mFactoryData.vendor_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.vendor_name.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(buf, mFactoryData.vendor_name.data, mFactoryData.vendor_name.len);
    buf[mFactoryData.vendor_name.len] = '\0';
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetVendorId(uint16_t & vendorId)
{
    ReturnErrorCodeIf(sizeof(vendorId) < mFactoryData.vendor_id.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.vendor_id.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&vendorId, mFactoryData.vendor_id.data, mFactoryData.vendor_id.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetProductName(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < mFactoryData.product_name.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.product_name.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memset(buf, 0, bufSize);
    memcpy(buf, mFactoryData.product_name.data, mFactoryData.product_name.len);
    buf[mFactoryData.product_name.len] = '\0';
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetProductId(uint16_t & productId)
{
    ReturnErrorCodeIf(sizeof(productId) < mFactoryData.product_id.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.product_id.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&productId, mFactoryData.product_id.data, mFactoryData.product_id.len);
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
    ReturnErrorCodeIf(bufSize < mFactoryData.serial_number.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.serial_number.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memset(buf, 0, bufSize);
    memcpy(buf, mFactoryData.serial_number.data, mFactoryData.serial_number.len);
    buf[mFactoryData.serial_number.len] = '\0';
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
{
    ReturnErrorCodeIf(!mFactoryData.manufacturing_date.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    uint8_t tmp[10] = { 0 };
    memcpy(tmp, mFactoryData.manufacturing_date.data, 10);
    year  = ((tmp[0] - 0x30) * 1000) + ((tmp[1] - 0x30) * 100) + ((tmp[2] - 0x30) * 10) + (tmp[3] - 0x30);
    month = ((tmp[5] - 0x30) * 10) + (tmp[6] - 0x30);
    day   = ((tmp[8] - 0x30) * 10) + (tmp[9] - 0x30);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetHardwareVersion(uint16_t & hardwareVersion)
{
    ReturnErrorCodeIf(sizeof(hardwareVersion) < mFactoryData.hw_ver.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.hw_ver.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(&hardwareVersion, mFactoryData.hw_ver.data, mFactoryData.hw_ver.len);
    return CHIP_NO_ERROR;
}
CHIP_ERROR FactoryDataProvider::GetHardwareVersionString(char * buf, size_t bufSize)
{
    ReturnErrorCodeIf(bufSize < mFactoryData.hw_ver_str.len + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.hw_ver_str.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memset(buf, 0, bufSize);
    memcpy(buf, mFactoryData.hw_ver_str.data, mFactoryData.hw_ver_str.len);
    buf[mFactoryData.hw_ver_str.len] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan)
{
    ReturnErrorCodeIf(uniqueIdSpan.size() < mFactoryData.rd_uniqueid.len, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!mFactoryData.rd_uniqueid.data, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    memcpy(uniqueIdSpan.data(), mFactoryData.rd_uniqueid.data, mFactoryData.rd_uniqueid.len);
    uniqueIdSpan.reduce_size(mFactoryData.rd_uniqueid.len);

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
