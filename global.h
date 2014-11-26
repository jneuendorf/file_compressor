#ifndef _GLOBAL
#define _GLOBAL

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

typedef unsigned char byte;
typedef unsigned long long number;
typedef signed long long signed_number;

// this is save because it's static inline
static inline bool is_big_endian() {
    const uint16_t endianness = 256;
    return *(const uint8_t *) &endianness;
}

#endif
