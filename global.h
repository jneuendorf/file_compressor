#ifndef _GLOBAL
#define _GLOBAL

#include <stdbool.h>

typedef unsigned char byte;
typedef unsigned long long number;

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
    number *diffs;
};

void init_nsb_data(struct nsb_data *nsb_data, number block_size);

#endif
