/*
 * Runtime subset of the Realtek Secure App ABI.
 * Keep this layout in sync with rtk_secure_service_protocol.h ABI version 1.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#define RTK_SECURE_DAC_ABI_VERSION 1u
#define RTK_SECURE_OP_SIGN_DAC_ATTESTATION 0x110u
#define RTK_SECURE_DAC_SIGNATURE_SIZE 64u
#define RTK_SECURE_DAC_STATUS_SUCCESS 0

typedef struct
{
    uint32_t abi_version;
    uint32_t struct_size;
    int32_t result;
} rtk_secure_dac_param_header_t;

typedef struct
{
    rtk_secure_dac_param_header_t header;
    uintptr_t message;
    uint32_t message_length;
    uintptr_t signature;
    uint32_t signature_size;
    uint32_t signature_length;
} rtk_secure_sign_dac_param_t;

#if defined(__cplusplus) && UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(rtk_secure_sign_dac_param_t) == 32, "Realtek Secure App DAC ABI layout mismatch");
#endif

#ifdef __cplusplus
extern "C" {
#endif

int32_t rtk_secure_service_nsc_call(uint32_t opcode, void * nonsecure_param, size_t param_size);

#ifdef __cplusplus
}
#endif

static inline void rtk_secure_sign_dac_param_init(rtk_secure_sign_dac_param_t * param)
{
    param->header.abi_version = RTK_SECURE_DAC_ABI_VERSION;
    param->header.struct_size = sizeof(*param);
    param->header.result      = -3; /* RTK_SECURE_STATUS_NOT_READY */
}
