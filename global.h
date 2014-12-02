#ifndef _GLOBAL
#define _GLOBAL

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DEBUG

#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif


typedef unsigned char byte;
typedef unsigned long long number;
typedef signed long long signed_number;

struct settings {
    bool verbose;
    bool compress;
    number max_perm_idx;
    number max_avg_idx;
    unsigned char block_size;
    unsigned char max_nsb;
    unsigned int memory_block_size; // how many to read into buffer for big files in mega bytes
};

struct settings settings;


// this is save because it's static inline
static inline bool is_big_endian() {
    const uint16_t endianness = 256;
    return *(const uint8_t *) &endianness;
}

#endif
