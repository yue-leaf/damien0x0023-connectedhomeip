/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                      Define to prevent recursive inclusion
 *============================================================================*/
#ifndef TRUSTZONE_DEMO_NSC_H
#define TRUSTZONE_DEMO_NSC_H

#include <stdint.h>
#include <stddef.h>

/** Maximum number of bytes accepted by trustzone_demo_nsc_process().
 *  Passing in_len > this value returns -1 immediately (no silent truncation). */
#define TRUSTZONE_DEMO_NSC_PROCESS_MAX_LEN   64U

/*============================================================================*
 *                              Functions
 *
 * These functions are Non-Secure Callable (NSC): they reside in Secure World
 * but are callable from Non-Secure World through the NSC memory region.
 *
 * The NSC region is a small "gateway" area configured by SAU/IDAU.  Each
 * function starts with an SG (Secure Gateway) instruction inserted by the
 * compiler (via NSC_ENTRY / __attribute__((cmse_nonsecure_entry))).
 * Branching directly into S memory without going through an SG instruction
 * triggers a SecureFault, preventing NS code from jumping to arbitrary
 * locations in the Secure World.
 *
 * Three calling patterns are provided as a learning reference:
 *
 *   trustzone_demo_nsc()          Mode 1 - no parameters
 *   trustzone_demo_nsc_add()      Mode 2 - value passing (scalars)
 *   trustzone_demo_nsc_process()  Mode 3 - pointer passing (with validation)
 *============================================================================*/

/**
 * @brief  Mode 1: No-parameter NSC call.
 *
 *         The simplest NSC pattern.  No arguments, no return value.
 *         Demonstrates the basic S/NS world transition via the veneer.
 *
 * @note   The NS caller must link against the CMSE import library
 *         (app_s_CMSE_Lib.o) so the linker can resolve the veneer stub.
 */
void trustzone_demo_nsc(void);

/**
 * @brief  Mode 2: Value-passing NSC call.
 *
 *         Integer arguments are passed in CPU registers (r0-r3) and are
 *         automatically isolated at the security boundary - no explicit
 *         pointer validation is required.  This is the safest and most
 *         efficient way to exchange data across the TrustZone boundary.
 *
 * @param  a   First operand.
 * @param  b   Second operand.
 * @return     a + b, computed inside Secure World.
 */
int32_t trustzone_demo_nsc_add(int32_t a, int32_t b);

/**
 * @brief  Mode 3: Pointer-passing NSC call.
 *
 *         Demonstrates the mandatory steps when NS pointers are passed to
 *         Secure World:
 *           1. Validate every NS pointer with cmse_check_address_range()
 *              before dereferencing it.
 *           2. Copy NS input data onto the secure stack (prevents TOCTOU).
 *           3. Process data entirely in secure memory.
 *           4. Write results back to the validated NS output buffer.
 *           5. Zero the secure stack buffer before returning.
 *
 * @param  ns_in    Pointer to input data in NS memory (read-only).
 *                  Must not point into Secure memory.
 * @param  in_len   Number of bytes to read from ns_in.
 * @param  ns_out   Pointer to output buffer in NS memory (write).
 *                  Must not point into Secure memory.
 * @param  out_len  Size of the ns_out buffer in bytes.
 * @return          Number of bytes written to ns_out, or -1 on error
 *                  (invalid pointer or zero-length).
 */
int32_t trustzone_demo_nsc_process(const uint8_t *ns_in,  size_t in_len,
                                   uint8_t       *ns_out, size_t out_len);

#endif /* TRUSTZONE_DEMO_NSC_H */
