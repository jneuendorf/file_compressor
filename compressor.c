#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "permutation.h"
#include "compressor.h"


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

// bool pattern_is_sorted(struct sequence *seq) {
//     char *chr;
//     char expected_char;
//     char current_chr;
//
//     // printf("pattern_is_sorted\n");
//
//     // point to 1st char
//     chr = seq->chars;
//
//     expected_char = '.'; // any
//
//     // iterate chars till zero byte
//     while((current_chr = *chr) != 0) {
//         // current_chr = *chr;
//         // printf("%d %c\n", chr, current_chr);
//         // expecting specific char
//         if(expected_char == '0' || expected_char == '1') {
//             if(current_chr != expected_char) {
//                 return false;
//             }
//             // else: do nothing...everything is ok
//         }
//         // 0 or 1 may be next
//         else {
//             if(current_chr == '1') {
//                 expected_char = '1';
//             }
//             // else: expect any => dont change expected_char's value
//         }
//         // go to next char
//         chr++;
//     }
//
//     return true;
// }


// bool pattern_is_sorted_inverse(struct sequence *seq) {
//     char *chr;
//     char expected_char;
//     char current_chr;
//
//     // printf("pattern_is_sorted_inverse\n");
//
//     // point to 1st char
//     chr = seq->chars;
//
//     expected_char = '.'; // any
//
//     // iterate chars till zero byte
//     while((current_chr = *chr) != 0) {
//         // current_chr = *chr;
//         // printf("%d %c\n", chr, current_chr);
//         // expecting specific char
//         if(expected_char == '0' || expected_char == '1') {
//             if(current_chr != expected_char) {
//                 return false;
//             }
//             // else: do nothing...everything is ok
//         }
//         // 0 or 1 may be next
//         else {
//             if(current_chr == '0') {
//                 expected_char = '0';
//             }
//             // else: expect any => dont change expected_char's value
//         }
//         // go to next char
//         chr++;
//     }
//
//     return true;
// }


bool pattern_is_sorted(number num) {
    // 1, 3, 7, 15, ... = 2^n-1
    // regex: 0*1*

    // while last bit is a 1 => shift right
    while((num & 1) == 1) {
        num = num >> 1;
    }

    // if no 1s are left it's 0!
    return num == 0;
}

bool pattern_is_sorted_inverse(number num) {
    // regex 1*0*

    printf("> %d\n", num);

    // while last bit is a 0 => shift right
    while((num & 1) == 0) {
        num = num >> 1;
        printf(">> %d\n", num);
    }

    //////////////////////////////
    // same as sorted!!! (TODO?)

    printf("> %d\n", num);

    // while last bit is a 1 => shift right
    while((num & 1) == 1) {
        num = num >> 1;
    }

    // if no 1s are left it's 0!
    return num == 0;
}


void define_patterns() {
    struct pattern sorted = {.matches = &pattern_is_sorted};
    struct pattern sorted_inverse = {.matches = &pattern_is_sorted_inverse};

    // allocate memory
    num_patterns = 2;
    patterns = calloc(num_patterns, sizeof(struct pattern));

    // assign patterns
    patterns[0] = sorted;
    patterns[1] = sorted_inverse;
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
