/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __COMPILER_ABSTRACTION_H__
#define __COMPILER_ABSTRACTION_H__

/** @brief Define compiler specific symbol */

#if defined ( __CC_ARM )
#define __STATIC_INLINE         static __inline
#define __STATIC_ALWAYS_INLINE  static __forceinline
#define barrier()               __memory_changed()
#define __UNUSED                __attribute__ ((unused))
#elif defined ( __ICCARM__ )
#define __STATIC_INLINE         static inline
#elif defined ( __GNUC__ )
#define __STATIC_INLINE         static inline
#define __STATIC_ALWAYS_INLINE  static inline __attribute__ ((always_inline))
#define barrier()               __asm volatile ("": : :"memory")
#define __UNUSED                __attribute__ ((unused))
#endif

#endif /* __COMPILER_ABSTRACTION_H__ */
