/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _UTILS_NS_H_
#define _UTILS_NS_H_

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                               Header Files
*============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "nsc/utils_nsc.h"

/** @defgroup PLATFORM_UTILS  Platform Utils
  * @brief
  * @{
  */

/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup PLATFORM_UTILS_Exported_Macros Platform Utils Exported Macros
  * @brief
  * @{
  */

#ifndef ABS
/** @brief Computes the absolute value of x. */
#define ABS(x)     (((x) > 0) ? (x) : ((~(x)) + 1))
#endif

/** @brief Computes the bitwise representation of 1 shifted left by _n places, for 64-bit integers. */
#define BIT64(_n)   (1ULL << (_n))

/** @brief Divides dividend by divisor and rounds the result to the nearest integer. */
#define DIVIDE_AND_ROUND(dividend, divisor)     (((dividend) + ((divisor) >> 1)) / (divisor))

/** @brief Divides dividend by divisor and rounds the result up to the nearest integer. */
#define DIVIDE_AND_ROUNDUP(dividend, divisor)   (((dividend) + ((divisor) - 1)) / (divisor))

/** @brief Computes the difference between two clock counts with a mask, preventing overflow. */
#define CLOCK_COUNT_DIFF(clk_begin, clk_end, bit_mask) \
    (((((uint64_t) clk_end & ((uint64_t) bit_mask - 1)) | (uint64_t) bit_mask) - \
      ((uint64_t) clk_begin & ((uint64_t) bit_mask - 1))) & ((uint64_t) bit_mask - 1))

/** @brief Base address for vendor counter access. */
#define VENDOR_COUNTER_BASE_ADDRESS 0x40006000

/** @brief Reads the value from a vendor-specific counter at Vendor_offset. */
#define VENDOR_COUNTER_READ( Vendor_offset ) \
    ((uint32_t)*((volatile uint32_t*)(VENDOR_COUNTER_BASE_ADDRESS + (Vendor_offset))))

/** @brief Writes the given Value to a vendor-specific counter at Vendor_offset. */
#define VENDOR_COUNTER_WRITE( Vendor_offset, Value ) \
    ((*((volatile uint32_t*)(VENDOR_COUNTER_BASE_ADDRESS + (Vendor_offset)))) = (Value))

/** @brief Constant representing a CPU counter bit number shifted by 32 bits. */
#define CPU_CNT_BIT_NUM             BIT64( 32 )

/** @brief Constant representing a PF RTC bit number shifted by 32 bits. */
#define PF_RTC_BIT_NUM              BIT64( 32 )

/** @brief Asserts x, calls vAssertHandler if x is 0. */
#define PLATFORM_ASSERT( x ) if( ( x ) == 0 ) { vAssertHandler( __FUNCTION__, __LINE__ ); }

/** @brief Triggers a build error if the expression e evaluates to non-zero. */
#define BUILD_BUG_ON_ZERO( e )  (sizeof(struct { int _; int:-!!(e); }) \
    - sizeof(struct { int _; }))

#ifndef SAME_TYPE
/** @brief Checks if the types of a and b are the same. */
#define SAME_TYPE( a, b )  __builtin_types_compatible_p(typeof( a ), typeof( b ))
#endif

/** @brief Ensures a is an array type, results in compile-time error if not. */
#define MUST_BE_ARRAY( a )  BUILD_BUG_ON_ZERO(SAME_TYPE(( a ), &(*a)))

#ifndef ARRAY_SIZE
/** @brief Computes the number of elements in array a. */
#define ARRAY_SIZE( a ) ((sizeof( a ) / sizeof(*a)) + MUST_BE_ARRAY( a ))
#endif

#ifndef hex2char
/** @brief Converts a nibble value to its corresponding hexadecimal character. */
#define hex2char( val ) ("0123456789abcdef"[(val) & 0xf])
#endif

/** @brief Counts the number of 1-bits in the integer x. */
#define count_1bits( x ) (uint8_t)__builtin_popcount( x )

/** @brief Counts the number of 0-bits in the integer x (assuming 32-bit integer). */
#define count_0bits( x ) (32 - count_1bits( x ))

/** @brief Checks if the inverse data is correct against the given mask. */
#define check_inverse( data, inverse_data, mask ) ((data ^ inverse_data) == mask)

/** @brief Sets the bits in val specified by mask. */
#define SET_BIT( val, mask )          ((val) |=  (mask))

/** @brief Clears the bits in val specified by mask. */
#define CLR_BIT( val, mask )          ((val)  &= ~(mask))

/** @brief Converts microseconds to CPU clock ticks, calculates residual in operation. */
#define US_TO_CPU_CLK( us, residual )\
    cal_quotient_remainder( 1000000, get_SystemCpuClock(), us, &residual )

/** @brief Converts CPU clock ticks to microseconds. */
#define CPU_CLK_TO_US( cpu_clk )\
    cal_quotient_remainder( get_SystemCpuClock(), 1000000, cpu_clk, NULL )

/** End of PLATFORM_UTILS_Exported_Macros
  * @}
  */

/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup PLATFORM_UTILS_Exported_Types Platform Utils Exported Types
  * @brief
  * @{
  */

typedef enum
{
    ROUND                   = 0,
    ROUNDUP                 = 1,
    ROUNDDOWN               = 2,
} RoundType;

typedef enum
{
    SYSTICK_EXTERNAL_CLOCK   = 0,
    SYSTICK_PROCESSOR_CLOCK  = 1
} SYSTICK_CLK_SRC_TYPE;

/** End of PLATFORM_UTILS_Exported_Types
  * @}
  */

/*============================================================================*
*                              Functions
*============================================================================*/
/** @defgroup PLATFORM_UTILS_Exported_Functions Platform Utils Exported Functions
  * @brief
  * @{
  */

#if   defined ( __CC_ARM )
/**
 * @brief Busy delay for specified millisecond
 * @param t   to specify t milliseconds to delay
 * @return none
 */
extern volatile void (*platform_delay_ms)(uint32_t t);
/**
 * @brief Busy delay for specified microsecond
 * @param t   to specify t microseconds to delay
 * @return none
 */
extern volatile void (*platform_delay_us)(uint32_t t);
#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) || defined (__GNUC__)
/**
 * @brief Busy delay for specified millisecond
 * @param t   to specify t milliseconds to delay
 * @return none
 */
extern void (*platform_delay_ms)(uint32_t t);
/**
 * @brief Busy delay for specified microsecond
 * @param t   to specify t microseconds to delay
 * @return none
 */
extern void (*platform_delay_us)(uint32_t t);
#endif

/**
 * @brief Calculates the quotient and remainder of two 64-bit integers.
 * @return The quotient.
 */
extern uint32_t (*cal_quotient_remainder)(const uint64_t, const uint64_t, const uint32_t,
                                          uint32_t *);

/**
 * @brief An empty function that performs no operations.
 */
void empty_function(void);

/**
 * @brief A function that always returns true.
 * @return true
 */
bool true_function(void);

/**
 * @brief Returns a void pointer.
 * @return A void pointer.
 */
void *void_ptr_function(void);

/**
 * @brief Handles assertion failures by reporting the function name and line number.
 * @param[in]  pFuncName  The name of the function where the assertion failed.
 * @param[in]  funcLine   The line number where the assertion failed.
 */
extern void vAssertHandler(const char *pFuncName, uint32_t funcLine);

/**
 * @brief Reverse the bits in a 32-bit word.
 * @param[in] num The 32-bit integer whose bits are to be reversed.
 * @return The 32-bit integer with reversed bits.
 */
uint32_t reverse_bits_in_word(uint32_t num);

/**
 * @brief Check if two ranges overlap.
 * @param[in] range0 An array of two 32-bit integers representing the first range.
 * @param[in] range1 An array of two 32-bit integers representing the second range.
 * @return true if the ranges overlap, false otherwise.
 */
bool is_overlapped(uint32_t range0[2], uint32_t range1[2]);

/**
 * @brief Calculate the difference between two clock counts considering a bit mask.
 * @param[in] clk_begin The starting clock count.
 * @param[in] clk_end The ending clock count.
 * @param[in] bit_mask The bit mask to apply to the clock counts.
 * @return The difference between the two clock counts.
 */
uint64_t clk_cnt_diff(uint64_t clk_begin, uint64_t clk_end, uint64_t bit_mask);

/**
 * @brief Compare two clock counts considering a third comparison clock count and a bit mask.
 * @param[in] clk_begin The base clock count.
 * @param[in] clk_end_1 The first clock count to compare.
 * @param[in] clk_end_2 The second clock count to compare.
 * @param[in] bit_mask The bit mask to apply to the clock counts.
 * @return true if clk_end_1 is closer to clk_begin than clk_end_2, false otherwise.
 */
bool clk_cnt_cmp(uint64_t clk_begin, uint64_t clk_end_1, uint64_t clk_end_2, uint64_t bit_mask);

/**
 * @brief Convert microseconds to RTC clock ticks.
 * @param[in] us The time in microseconds.
 * @param[in] round_type The rounding type to apply (e.g., round up or down).
 * @return The equivalent time in RTC clock ticks.
 */
uint32_t us_to_rtc_clk(uint32_t us, RoundType round_type);

/**
 * @brief Convert RTC clock ticks to microseconds.
 * @param[in] rtc_clk The time in RTC clock ticks.
 * @param[in] round_type The rounding type to apply (e.g., round up or down).
 * @return The equivalent time in microseconds.
 */
uint32_t rtc_clk_to_us(uint32_t rtc_clk, RoundType round_type);

/**
 * @brief Initialize the DWT cycle counter.
 */
void dwt_cyccnt_init(void);

/**
 * @brief Read the CPU counter value.
 * @return The current value of the CPU counter.
 */
uint32_t read_cpu_counter(void);

/**
 * @brief Get the source type of the system tick clock.
 * @return The source type of the system tick clock.
 */
SYSTICK_CLK_SRC_TYPE get_sys_tick_clk_type(void);

/**
 * @brief Get the current system core clock frequency.
 * @return The system core clock frequency in Hz.
 */
uint32_t get_SystemCoreClock(void);

/**
 * @brief Get the current system CPU clock frequency.
 * @return The system CPU clock frequency in Hz.
 */
extern uint32_t (*get_SystemCpuClock)(void);

/**
 * @brief Function pointer to get the stack enable status.
 * @return The stack enable status.
 */
extern uint32_t (*get_stack_en)(void);

/**
 * @brief Update the CPU tick to microsecond conversion factor based on the CPU frequency.
 * @param cpu_freq The CPU frequency in Hz.
 */
void update_cpu_tick_to_us(uint32_t cpu_freq);


#define IS_USE_VHCI (get_stack_en())

/**
 * @brief Generate random number given max number allowed
 * @param[in] max   to specify max number that is allowed
 * @return random number
 */

uint32_t platform_random(uint32_t max);

/** End of PLATFORM_UTILS_Exported_Functions
  * @}
  */

/** End of PLATFORM_UTILS
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif
