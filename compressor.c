#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "permutation.h"
#include "compressor.h"




void init_nsb_data(struct nsb_data *nsb_data, number num_blocks) {
    // TODO: arrays might be way too big! init them with the expected size of num_blocks / (block_size + 1)
    nsb_data->indices = calloc(num_blocks, sizeof(number));
    nsb_data->perm_indices = calloc(num_blocks, sizeof(number));
    nsb_data->diffs = calloc(num_blocks, sizeof(signed_number));
}


// from http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
unsigned int number_of_set_bits(int i) {
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

char* bits_to_string(void *p, unsigned int bytes) {
    signed int i, j;
    // int lsb; // least significant bit
    char* res;
    // unsigned long long bits;
    char* byte_p;
    char byte;

    // cast void pointer to long long pointer and dereference
    // bits = *((unsigned long long *) p);
    byte_p = (char *) p;

    // allocate + clear
    res = calloc(bytes * 8 + 1, sizeof(char));

    if(res != NULL) {
        // iterate through bytes
        for(j = 0; j < bytes; ++j) {
            byte = *byte_p;
            // printf("j=%d\n", j);
            // loop backwards
            for(i = 7; i >= 0; --i) {
                // printf(" 8*j + i=%d\n", 8*j+i);
                // printf("i=%d, bit=%d\n", i, lsb);
                // lsb is 1?
                if((byte & 1) == 1) {
                    res[8*j + i] = '1';
                }
                else {
                    res[8*j + i] = '0';
                }
                // drop lsb
                byte = byte >> 1;
            }

            // point to next char
            byte_p++;
        }
    }

    return res;
}

/*
*
*/

struct bit_stream create_bit_stream(number *n, unsigned char used_bits) {
    struct bit_stream result = { .bits = n, .num_blocks = 1, .used_bits = used_bits};
    return result;
}

void append_to_bit_stream(struct bit_stream *bit_stream, struct bit_stream *block) {
    // if the used bits of both are less than sizeof(number) => just append
    if(bit_stream->used_bits + block->used_bits < sizeof(number)) {

    }
    // not enough space => we need to create a second block
    else {

    }
}

void compress() {

}

void compress_blockwise() {

}

// int is_sorted(struct sequence *seq) {
//     return seq->length;
// }

// http://graphics.stanford.edu/~seander/bithacks.html#SwappingBitsXOR
// Swapping individual bits with XOR

// unsigned int i, j; // positions of bit sequences to swap
// unsigned int n;    // number of consecutive bits in each sequence
// unsigned int b;    // bits to swap reside in b
// unsigned int r;    // bit-swapped result goes here
//
// unsigned int x = ((b >> i) ^ (b >> j)) & ((1U << n) - 1); // XOR temporary
// r = b ^ ((x << i) | (x << j));
// As an example of swapping ranges of bits suppose we have have b = 00101111 (expressed in binary) and we want to swap the n = 3 consecutive bits starting at i = 1 (the second bit from the right) with the 3 consecutive bits starting at j = 5; the result would be r = 11100011 (binary).
// This method of swapping is similar to the general purpose XOR swap trick, but intended for operating on individual bits.  The variable x stores the result of XORing the pairs of bit values we want to swap, and then the bits are set to the result of themselves XORed with x.  Of course, the result is undefined if the sequences overlap.
