#include "compress.h"


void read_data(FILE *file, struct nsb_data *nsb_datas, number block_size, number num_blocks, number max_nsb, number **nsb_order_p, number *nsb_arrays_lengths, number **nsb_permutations) {
    char            *buffer;
    number          block_index;
    number          current_nsb;
    number          perm_index;
    number          *nsb_order;
    number          *block;
    struct nsb_data current_nsb_data;

    buffer = calloc(block_size / 8 + 1, sizeof(char));

    // set array 'nsb_arrays_lengths' to zeros
    memset(nsb_arrays_lengths, 0, max_nsb * sizeof(number));
    block_index = 0;

    nsb_order = calloc(num_blocks, sizeof(number));
    // point to allocated memory
    *nsb_order_p = nsb_order;

    // read file til its end (-> feof = end of file)
    while(!feof(file)) {
        fread(buffer, block_size / 8, 1, file);

        // D(printf("buffer: %s (%lu)\n", buffer, strlen(buffer));)
        D(printf("buffer: %x,%x (%llu bytes)\n", (unsigned short) *buffer, (unsigned short) buffer[1], block_size / 8);)

        block = (number *) buffer;
        // D(printf("%d\n", *block);)

        current_nsb = (number) number_of_set_bits(*block); // TODO: for now, only works for 32 bit integers

        // D(printf("current nsb = %llu in %llu = %s\n", current_nsb, *((number *) buffer), bits_to_string(buffer, block_size));)
        // D(printf("current nsb = %llu @ %llu\n", current_nsb, block_index);)

        current_nsb_data = nsb_datas[current_nsb];

        current_nsb_data.indices[nsb_arrays_lengths[current_nsb]] = block_index;

        // keep track of the order of blocks in our data structure
        nsb_order[block_index] = current_nsb;

        // get permutation index of current block
        perm_index = 0;
        while(nsb_permutations[current_nsb][perm_index++] != *block) {}

        current_nsb_data.perm_indices[nsb_arrays_lengths[current_nsb]] = perm_index;

        // diffs can be calculated after the average has been determined (after the whole file has been read!)
        nsb_arrays_lengths[current_nsb]++; // an element was pushed
        // TODO: move increment to last use of block_index
        block_index++; // block done
    }

}
