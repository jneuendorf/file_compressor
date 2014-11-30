#ifndef _COMPRESSOR
#define _COMPRESSOR

#include "global.h"



#define NEEDED_BITS(n) ((n) > 0 ? (floor(log2(n)) + 1) : 1)
// #define CEIL(x, y) (1 + ((x - 1) / y))
#define CEIL_X_DIV_Y(x,y) (((x) + (y) - 1) / (y))


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
    number max_diff_bits;
    // number max_diff;
};

struct bit_stream {
    number *bits; // array of numbers with 64 bits each
    number num_blocks; // array length
    number *last_block; // points to the current (last used) element of the array
    unsigned char avail_bits; // available bits
};



// SIGNATURES
void init_nsb_data(struct nsb_data *nsb_data, number block_size);

// BIT OPERATIONS
unsigned int number_of_set_bits(int i);
char* bits_to_string(void *p, unsigned int relevant_bits);

// BIT STREAM OPERATIONS
struct bit_stream create_bs(number n, unsigned char used_bits);
void append_num_to_bs(struct bit_stream *bit_stream, number *block, unsigned char used_bits);
unsigned char* bs_to_byte_stream(struct bit_stream *bit_stream, number *written_bytes);
number get_bs_size(struct bit_stream *bit_stream);

#endif
