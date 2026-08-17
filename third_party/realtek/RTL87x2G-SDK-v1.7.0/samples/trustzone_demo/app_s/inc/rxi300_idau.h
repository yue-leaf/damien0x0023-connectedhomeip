/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef RXI300_IDAU_H
#define RXI300_IDAU_H

#include "rtl876x.h"

typedef struct
{
    /* RXI-300 Design Information registers */
    __I uint32_t rsvd0[64];

    /* RXI-300 IDAU registers */

    /* base address of non-secure region 0, fixed at 0x40000000 */
    __I uint32_t IDAU_BA0;

    /* top address of non-secure region 0, fixed at 0x4FFFFFFF */
    __I uint32_t IDAU_LA0;

    union
    {
        __IO uint32_t IDAU_BA1; /* base address of non-secure region 1 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t base_addr_region1: 20;
        } BITS_108;
    } u_108;
    union
    {
        __IO uint32_t IDAU_LA1; /* top address of non-secure region 1 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t top_addr_region1: 20;
        } BITS_10C;
    } u_10C;
    union
    {
        __IO uint32_t IDAU_BA2; /* base address of non-secure region 2 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t base_addr_region2: 20;
        } BITS_110;
    } u_110;
    union
    {
        __IO uint32_t IDAU_LA2; /* top address of non-secure region 2 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t top_addr_region2: 20;
        } BITS_114;
    } u_114;
    union
    {
        __IO uint32_t IDAU_BA3; /* base address of non-secure region 3 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t base_addr_region3: 20;
        } BITS_118;
    } u_118;
    union
    {
        __IO uint32_t IDAU_LA3; /* top address of non-secure region 3 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t top_addr_region3: 20;
        } BITS_11C;
    } u_11C;
    union
    {
        __IO uint32_t IDAU_BA4; /* base address of non-secure region 4 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t base_addr_region4: 20;
        } BITS_120;
    } u_120;
    union
    {
        __IO uint32_t IDAU_LA4; /* top address of non-secure region 4 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t top_addr_region4: 20;
        } BITS_124;
    } u_124;
    union
    {
        __IO uint32_t IDAU_BA5; /* base address of non-secure region 5 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t base_addr_region5: 20;
        } BITS_128;
    } u_128;
    union
    {
        __IO uint32_t IDAU_LA5; /* top address of non-secure region 5 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t top_addr_region5: 20;
        } BITS_12C;
    } u_12C;
    union
    {
        __IO uint32_t IDAU_BA6; /* base address of non-secure region 6 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t base_addr_region6: 20;
        } BITS_130;
    } u_130;
    union
    {
        __IO uint32_t IDAU_LA6; /* top address of non-secure region 6 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t top_addr_region6: 20;
        } BITS_134;
    } u_134;
    union
    {
        __IO uint32_t IDAU_BA7; /* base address of non-secure region 7 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t base_addr_region7: 20;
        } BITS_138;
    } u_138;
    union
    {
        __IO uint32_t IDAU_LA7; /* top address of non-secure region 7 */
        struct
        {
            __IO uint32_t rsvd: 12;
            __IO uint32_t top_addr_region7: 20;
        } BITS_13C;
    } u_13C;
    union
    {
        __IO uint32_t IDAU_CTRL; /* region entry enable */
        struct
        {
            __I uint32_t region0_enable: 1;  /* enable entry for BA0/LA0, always be enabled */
            __IO uint32_t region1_enable: 1; /* enable entry for BA1/LA1 */
            __IO uint32_t region2_enable: 1; /* enable entry for BA2/LA2 */
            __IO uint32_t region3_enable: 1; /* enable entry for BA3/LA3 */
            __IO uint32_t region4_enable: 1; /* enable entry for BA4/LA4 */
            __IO uint32_t region5_enable: 1; /* enable entry for BA5/LA5 */
            __IO uint32_t region6_enable: 1; /* enable entry for BA6/LA6 */
            __IO uint32_t region7_enable: 1; /* enable entry for BA7/LA7 */
            __IO uint32_t rsvd: 24;
        } BITS_140;
    } u_140;
    union
    {
        __IO uint32_t IDAU_LOCK; /* lock write transfer for all IDAU register
                                  * (including lock register) until system reset */
        struct
        {
            __IO uint32_t lock: 1;    /* 0: disable lock, 1: enable lock */
            __IO uint32_t rsvd: 31;
        } BITS_144;
    } u_144;

} RXI300_S_Typedef;


#define RXI300_REG_BASE                    0x50004000UL
#define RXI300              ((RXI300_S_Typedef  *) RXI300_REG_BASE)


#endif // __RXI300_IDAU_H_