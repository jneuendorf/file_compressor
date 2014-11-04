#ifndef _COMPRESSOR
#define _COMPRESSOR

#include "global.h"

// struct sequence {
//     char* chars;
//     unsigned int length;
//     unsigned char pattern_id;
//     // int (*pattern)(int,int);
// };

struct sequence {
    number n;
    byte pattern_id;
};

// PATTERNS
struct pattern {
    bool (*matches)(struct sequence*); // check if sequence matches pattern
};

struct pattern *patterns;
unsigned int num_patterns;

// signatures
void define_patterns();
unsigned int number_of_set_bits(int i);
char* bits_to_string(void *p, unsigned int relevant_bits);

bool pattern_is_sorted(number num);
bool pattern_is_sorted_inverse(number num);


#endif
