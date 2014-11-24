#ifndef _COMPRESSOR
#define _COMPRESSOR

#include "global.h"

struct mapper_entry {
    number nsb;
    number average;
    number negative_diffs;
    number max_diff;
};

struct nsb_data {
    number *indices;
    number *perm_indices;
    number avg;
    signed_number *diffs;
};

struct bit_stream {
    number *bits; // array of numbers with 64 bits each
    number num_blocks;
    unsigned char used_bits; // used bits (in case some zeros are to be ignored)
};


void init_nsb_data(struct nsb_data *nsb_data, number block_size);

// signatures
// void define_patterns();
unsigned int number_of_set_bits(int i);
char* bits_to_string(void *p, unsigned int relevant_bits);

// bool pattern_is_sorted(number num);
// bool pattern_is_sorted_inverse(number num);


#endif
