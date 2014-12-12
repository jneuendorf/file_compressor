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
    unsigned char block_size;
    unsigned char data_perm_block_size; // how many (compressed) data blocks to permute
    unsigned int memory_block_size; // how many to read into buffer for big files in mega bytes
} settings;

struct compress_data {
    number max_perm_idx;
    number max_avg_idx;
    number num_blocks;
    unsigned char block_size;
    unsigned char max_nsb;
} compress_data;

struct decompress_data {
    // number max_avg_idx;
    // number max_perm_idx;
    unsigned char block_size;
    unsigned char max_nsb;
    unsigned char num_mapper_entries;
    unsigned char max_used_nsb;
} decompress_data;

// this is save because it's static inline
static inline bool is_big_endian() {
    const uint16_t endianness = 256;
    return *(const uint8_t *) &endianness;
}


// SIGNATURES
number binom(unsigned char n, unsigned char k);
void parse_cmd_line_arguments(int argc, char const *argv[], struct settings *settings);

#endif
