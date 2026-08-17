#ifndef SD_UTILS_H
#define SD_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#define STATIC_ASSERT(e)  typedef char StaticAssertOn##__LINE__[(e) ? 1 : -1]

#define IS_ADDR_ALIGNED(Addr)  ((uint32_t)(Addr) % 4 == 0)

#define MIN2(a, b)  ((a) < (b) ? (a) : (b))


static inline uint32_t BitsToU32(const void *pBits, uint32_t FirstBit, uint32_t LastBit)
{
    uint32_t FirstByte = FirstBit / 8;
    uint32_t FirstOfs = FirstBit % 8;
    uint32_t LastByte = LastBit / 8;

    uint32_t ByteCnt = LastByte + 1 - FirstByte;
    uint32_t BitsCnt = LastBit + 1 - FirstBit;
    ASSERT(BitsCnt <= 32);

    uint64_t u64;
    const uint8_t *p = pBits;
    memcpy(&u64, p + FirstByte, ByteCnt);

    return (uint32_t)(u64 >> FirstOfs) & (~0UL >> (32 - BitsCnt));
}

#endif /* SD_UTILS_H */
