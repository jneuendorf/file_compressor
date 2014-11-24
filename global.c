#include <stdlib.h>

#include "global.h"

void init_nsb_data(struct nsb_data *nsb_data, number block_size) {
    nsb_data->indices = calloc(block_size / 8, sizeof(number));
    nsb_data->perm_indices = calloc(block_size / 8, sizeof(number));
    nsb_data->diffs = calloc(block_size / 8, sizeof(number));
}
