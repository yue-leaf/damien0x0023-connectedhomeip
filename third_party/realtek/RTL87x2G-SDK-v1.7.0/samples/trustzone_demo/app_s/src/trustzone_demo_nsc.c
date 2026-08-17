/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl876x.h"
#include "trace.h"
#include "nsc_veneer.h"
#include "trustzone_demo_nsc.h"
#include <arm_cmse.h>
#include <string.h>

/*============================================================================*
 *                              NSC Functions
 *
 * NSC_ENTRY is equivalent to __attribute__((cmse_nonsecure_entry)).
 * The compiler inserts an SG (Secure Gateway) instruction at the entry of
 * each NSC function. The function must reside in the NSC memory region
 * configured by SAU/IDAU, otherwise a SecureFault will be triggered.
 *
 * Three NSC calling patterns are demonstrated here:
 *   Mode 1 - No parameters    : simplest form, no pointer validation needed
 *   Mode 2 - Value passing    : integer args passed via registers, safe by design
 *   Mode 3 - Pointer passing  : NS pointers MUST be validated before use
 *============================================================================*/

/*-----------------------------------------------------------*/
/*
 * Mode 1: No-parameter NSC call (simplest form)
 *
 * Key points:
 *   - NSC_ENTRY marks this as a Secure Gateway entry point
 *   - The SG instruction at entry verifies the call came through the veneer
 *   - No pointer validation needed since there are no pointer arguments
 */
NSC_ENTRY
void trustzone_demo_nsc(void)
{
    DBG_DIRECT("[S] trustzone_demo_nsc: called from NS world");
}

/*-----------------------------------------------------------*/
/*
 * Mode 2: Value-passing NSC call
 *
 * Key points:
 *   - Integer/scalar arguments are passed via CPU registers (r0-r3)
 *   - No pointer validation needed; values are copied into S-side registers
 *     at the security boundary automatically by the hardware
 *   - This is the safest and most efficient NSC calling pattern
 *
 * @param  a   First operand (passed by value, safe)
 * @param  b   Second operand (passed by value, safe)
 * @return     Sum computed in Secure World
 */
NSC_ENTRY
int32_t trustzone_demo_nsc_add(int32_t a, int32_t b)
{
    int32_t result = a + b;
    DBG_DIRECT("[S] secure_add(%d, %d) = %d", a, b, result);
    return result;
}

/*-----------------------------------------------------------*/
/*
 * Mode 3: Pointer-passing NSC call
 *
 * Key points:
 *   - NS pointers MUST be validated with cmse_check_address_range() before
 *     any dereference; an unvalidated NS pointer could point into S memory
 *     and leak or corrupt secure data
 *   - After validation, copy NS data onto the secure stack before processing
 *     to prevent TOCTOU (Time-Of-Check-Time-Of-Use) attacks: the NS world
 *     could modify the buffer between the check and the actual use
 *   - in_len is capped at TRUSTZONE_DEMO_NSC_PROCESS_MAX_LEN; callers
 *     passing a larger length receive -1 (not silent truncation)
 *   - Clear the secure stack buffer after use to avoid leaking data
 *
 * @param  ns_in    Input buffer in NS memory (read-only)
 * @param  in_len   Length of input buffer in bytes (max TRUSTZONE_DEMO_NSC_PROCESS_MAX_LEN)
 * @param  ns_out   Output buffer in NS memory (write)
 * @param  out_len  Length of output buffer in bytes
 * @return          Number of bytes written, or -1 on error
 */
NSC_ENTRY
int32_t trustzone_demo_nsc_process(
    const uint8_t *ns_in,  size_t in_len,
    uint8_t       *ns_out, size_t out_len)
{
    /* Step 1: Reject zero-length or oversized requests explicitly.
     *
     * Returning an error (rather than silently clamping) lets the caller
     * detect misuse at development time instead of getting wrong results.
     */
    if (in_len == 0 || in_len > TRUSTZONE_DEMO_NSC_PROCESS_MAX_LEN)
    {
        DBG_DIRECT("[S] ERROR: in_len=%d invalid (valid range: 1-%d)",
                   (int)in_len, TRUSTZONE_DEMO_NSC_PROCESS_MAX_LEN);
        return -1;
    }
    if (out_len < in_len)
    {
        DBG_DIRECT("[S] ERROR: out_len=%d < in_len=%d", (int)out_len, (int)in_len);
        return -1;
    }

    /* Step 2: Validate NS pointers before any dereference.
     *
     * cmse_check_address_range() checks that the entire range [ptr, ptr+size)
     * lies within NS (or NSC) memory as defined by SAU/IDAU.
     * If the pointer points into S memory, this returns NULL and we reject it.
     *
     * CMSE_NONSECURE | CMSE_MPU_READ      : range must be NS-readable
     * CMSE_NONSECURE | CMSE_MPU_READWRITE : range must be NS-writable
     */
    if (cmse_check_address_range((void *)ns_in, in_len,
                                 CMSE_NONSECURE | CMSE_MPU_READ) == NULL)
    {
        DBG_DIRECT("[S] ERROR: ns_in pointer not in NS memory");
        return -1;
    }
    if (cmse_check_address_range((void *)ns_out, out_len,
                                 CMSE_NONSECURE | CMSE_MPU_READWRITE) == NULL)
    {
        DBG_DIRECT("[S] ERROR: ns_out pointer not in NS memory");
        return -1;
    }

    /* Step 3: Copy NS input onto the secure stack before processing.
     *
     * Even after cmse_check_address_range() passes, the NS world runs
     * concurrently (e.g. via interrupts) and could modify ns_in between
     * the check and the actual read - a TOCTOU race condition.
     * Copying to a local secure buffer eliminates this window.
     */
    uint8_t secure_buf[TRUSTZONE_DEMO_NSC_PROCESS_MAX_LEN];
    memcpy(secure_buf, ns_in, in_len);

    /* Step 4: Process data entirely within secure memory.
     * (XOR with 0xAA as a simple placeholder transformation)
     */
    for (size_t i = 0; i < in_len; i++)
    {
        ns_out[i] = secure_buf[i] ^ 0xAAU;
    }
    DBG_DIRECT("[S] processed %d bytes from NS world", (int)in_len);

    /* Step 5: Clear the secure stack buffer to avoid leaking data
     * if the stack is later inspected (e.g. via a debugger or fault dump).
     */
    memset(secure_buf, 0, sizeof(secure_buf));

    return (int32_t)in_len;
}

/*-----------------------------------------------------------*/
