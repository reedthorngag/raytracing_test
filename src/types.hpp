#include <stdint.h>

#ifndef _TYPES
#define _TYPES

const int RGB_MASK = (1 << 21) - 1;
#define RGB_RANGE RGB_MASK
#define convertScale(x) ((u64)((float)x/255.0 * RGB_RANGE) & RGB_MASK)
#define RGB_TO_U64(r,g,b) ((convertScale(r) << 42) | (convertScale(g) << 21) | convertScale(b))

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t

#endif

