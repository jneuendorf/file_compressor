#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////
#include "global.h"
#include "permutation.h"
#include "compressor.h"

//////////////////////////////////////////////////////////////////
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 16
#endif
// TODO: make this dynamic!
#ifndef MAX_NSB
#define MAX_NSB (BLOCK_SIZE + 1)
#endif
#ifndef MAX_POSSIBLE_PERM_IDX
// #define MAX_POSSIBLE_PERM_IDX 70
#define MAX_POSSIBLE_PERM_IDX 12870 // binom(16 8)
#endif
#ifndef MAX_AVG_IDX
#define MAX_AVG_IDX (MAX_POSSIBLE_PERM_IDX / 2)
#endif


//////////////////////////////////////////////////////////////////


int main (int argc, char const *argv[]) {

    D(printf("endianness = %d\n", is_big_endian()));

    // BITSTREAM TEST

    number test = 1199;
    number test2 = 1199;
    struct bit_stream bs1;
    struct bit_stream bs2;
    bs1 = create_bs(test, 11); // 1199 = 0000000000000000000000000000000000000000000000000000010010101111
    // sould equal 10799631906434449408 (11 bits shifted to front and filled up with zeros)
    // 1001010111100000000000000000000000000000000000000000000000000000
    D(printf("> %llu\n", *(bs1.last_block)));
    // prints      10799631906434449408
    // bs2 = create_bs(&test2, 11);
    append_num_to_bs(&bs1, &test2, 11);
    append_num_to_bs(&bs1, &test2, 11);
    append_num_to_bs(&bs1, &test2, 11);
    append_num_to_bs(&bs1, &test2, 11);
    append_num_to_bs(&bs1, &test2, 11);

    // should print:
    // 10804907740292013867,
    // 13835058055282163712
    // 1001010111110010101111100101011111001010111110010101111100101011, 1100000000000000000000000000000000000000000000000000000000000000
    // correct!!

    number *p = bs1.bits;
    D(printf("Printing bocks:\n"));
    D(printf(">>> %llu\n", *p));
    while(p != bs1.last_block) {
        D(printf(">>> %llu\n", *(++p)));
    }

    // D(printf("> avail: %llu\n", bs1.avail_bits); // correct)!
    number num_bytes = 0;
    unsigned char *bytes = bs_to_byte_stream(&bs1, &num_bytes);
    unsigned char *p2 = bytes;

    D(printf("--> %s\n", bits_to_string(bytes, num_bytes)));

    for(int i = 0; i < num_bytes; ++i) {
    D(printf("%d\t->\t%d\t-\t%s\n",i , bytes[i], bits_to_string(p2++, 1)));
    }
    // prints 9 bytes copied, values:
    // 0
    // 0
    // 80
    // 235
    // 136
    // 127
    // 0
    // 0
    // 2
    // expected:
    // 100101011111001010111110010101111100101011111001010111110010101111000000
    // => 9 bytes is correct!
    // values: 149, 242, 190, 87, 202, 249, 95, 43, 192

    return 0;
}
