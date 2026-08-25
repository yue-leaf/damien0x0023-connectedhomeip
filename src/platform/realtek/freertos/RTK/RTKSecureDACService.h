#pragma once

#if defined(RTK_SECURE_DAC_SERVICE_HEADER)
#include RTK_SECURE_DAC_SERVICE_HEADER
#else
#error "Persistent DAC builds require RTK_SECURE_DAC_SERVICE_HEADER from the reviewed Realtek Secure App"
#endif

#ifndef RTK_SECURE_DAC_ABI_VERSION
#define RTK_SECURE_DAC_ABI_VERSION 1u
#endif
#ifndef RTK_SECURE_OP_SIGN_DAC_ATTESTATION
#define RTK_SECURE_OP_SIGN_DAC_ATTESTATION 0x110u
#endif
#ifndef RTK_SECURE_DAC_SIGNATURE_SIZE
#define RTK_SECURE_DAC_SIGNATURE_SIZE 64u
#endif
#ifndef RTK_SECURE_DAC_STATUS_SUCCESS
#define RTK_SECURE_DAC_STATUS_SUCCESS 0
#endif

static inline void rtk_secure_sign_dac_param_init(rtk_secure_sign_dac_param_t * param)
{
    param->header.abi_version = RTK_SECURE_DAC_ABI_VERSION;
    param->header.struct_size = sizeof(*param);
    param->header.result      = -3; /* RTK_SECURE_STATUS_NOT_READY */
}
