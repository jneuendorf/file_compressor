#include "compress.h"


void read_data(FILE *file, struct nsb_data *nsb_datas, number num_blocks, number **nsb_order_p, number *nsb_arrays_lengths, number **nsb_permutations) {
    char            *buffer;
    number          block_index;
    number          current_nsb;
    number          perm_idx;
    number          *nsb_order;
    number          *block;
    struct nsb_data current_nsb_data;

    buffer = calloc(settings.block_size / 8 + 1, sizeof(char));

    // set array 'nsb_arrays_lengths' to zeros
    memset(nsb_arrays_lengths, 0, settings.max_nsb * sizeof(number));
    block_index = 0;

    nsb_order = calloc(num_blocks, sizeof(number));
    // point to allocated memory
    *nsb_order_p = nsb_order;

    // read file til its end (-> feof = end of file)
    while(!feof(file)) {
        fread(buffer, settings.block_size / 8, 1, file);

        // D(printf("buffer: %x,%x (%llu bytes)\n", (unsigned short) *buffer, (unsigned short) buffer[1], settings.block_size / 8);)

        block = (number *) buffer;
        // D(printf("%d\n", *block);)

        current_nsb = (number) number_of_set_bits(*block); // TODO: for now, only works for 32 bit integers

        // D(printf("current nsb = %llu in %llu = %s\n", current_nsb, *((number *) buffer), bits_to_string(buffer, settings.block_size));)
        // D(printf("current nsb = %llu @ %llu\n", current_nsb, block_index);)

        current_nsb_data = nsb_datas[current_nsb];

        current_nsb_data.indices[nsb_arrays_lengths[current_nsb]] = block_index;

        // keep track of the order of blocks in our data structure
        nsb_order[block_index] = current_nsb;

        // get permutation index of current block
        perm_idx = 0;
        while(nsb_permutations[current_nsb][perm_idx++] != *block) {}

        current_nsb_data.perm_indices[nsb_arrays_lengths[current_nsb]] = perm_idx;

        // diffs can be calculated after the average has been determined (after the whole file has been read!)
        nsb_arrays_lengths[current_nsb]++; // an element was pushed
        // TODO: move increment to last use of block_index
        block_index++; // block done
    }
}


void calc_rem_data(struct nsb_data *nsb_datas, number *max_used_nsb, number *nsb_arrays_lengths) {
    number          min_perm_idx;
    number          max_perm_idx;
    number          avg_perm_idx;
    number          max_diff_bits;
    number          i;
    number          nsb;
    number          perm_idx;
    number          nsb_array_length;
    signed_number   diff;
    struct nsb_data current_nsb_data;

    *max_used_nsb = 0;
    // max_used_avg_idx = 0;
    for(nsb = 0; nsb < settings.max_nsb; ++nsb) {
        // cache variables
        nsb_array_length = nsb_arrays_lengths[nsb];
        current_nsb_data = nsb_datas[nsb];

        // get minimal and maximal permutation indices and maximal nsb with data
        // there is data for that nsb
        if(nsb_array_length > 0) {
            // current nsb is the current max. nsb
            *max_used_nsb = nsb;

            // get min and max
            min_perm_idx = settings.max_perm_idx;
            max_perm_idx = 0;
            for(i = 0; i < nsb_array_length; ++i) {
                perm_idx = current_nsb_data.perm_indices[i];
                if(perm_idx < min_perm_idx) {
                    min_perm_idx = perm_idx;
                }
                else if(perm_idx > max_perm_idx) {
                    max_perm_idx = perm_idx;
                }
            }
            // if only min_perm_idx has been set, max_perm_idx is unchanged (because of if-else)
            if(max_perm_idx < min_perm_idx) {
                max_perm_idx = min_perm_idx;
            }
        }
        else {
            min_perm_idx = 0;
            max_perm_idx = 0;
        }

        // calculate average permutation index (of min and max)
        // floor() is implicit because avg_perm_idx is integer
        avg_perm_idx = min_perm_idx + (max_perm_idx - min_perm_idx) / 2;

        nsb_datas[nsb].avg = avg_perm_idx;

        // get differences of perm. indices to the average perm. index (perm_idx - avg_perm_idx)
        max_diff_bits = 0;
        for(i = 0; i < nsb_array_length; ++i) {
            diff = current_nsb_data.perm_indices[i] - avg_perm_idx;
            current_nsb_data.diffs[i] = diff;

            // get absolute value of diff
            if(diff < 0) {
                diff = -diff;
            }
            // get number of bits we need for the absolute value of the diff
            if((diff = NEEDED_BITS((number) diff)) > max_diff_bits) {
                max_diff_bits = diff;
            }
        }
        // add sign bit
        max_diff_bits++;

        // now we know the max. number of bits we need for the absolute value of all diffs.
        // so now we transform the 2's complement of a negative diff to sign-magnitude (-> vorzeichen-bit)
        // this causes the negative diffs to take a lot less bits!!
        for(i = 0; i < nsb_array_length; ++i) {
            diff = current_nsb_data.diffs[i];
            // if diff is negative take care of creating a bit pattern we interpret as negative because we don't wanna use 2's complement because this needs as many bits as the data type it is saved in
            // so in that case we add '1' to the front (most significant bit)
            if(diff < 0) {
                // make positive
                diff = -diff;
                // OR it with a number that has
                diff = diff | (1 << (max_diff_bits - 1));
                // write back to data
                current_nsb_data.diffs[i] = diff;
            }
        }
        nsb_datas[nsb].max_diff_bits = max_diff_bits;
    }
}


void write_data_to_bit_stream(struct bit_stream *bit_stream, struct nsb_data *nsb_datas, number num_blocks, number max_used_nsb, number *nsb_order, number *nsb_arrays_lengths, number *nsb_offsets) {
    number          diff;
    number          i;
    number          max_avg_idx_bits;
    number          max_nsb_bits;
    number          max_used_nsb_bits;
    number          nsb;
    number          num_mapper_entries;
    struct nsb_data current_nsb_data;

    // get number of mapper entries (before anything else happens)
    num_mapper_entries = 0;
    for(nsb = 0; nsb < settings.max_nsb; ++nsb) {
        if(nsb_arrays_lengths[nsb] > 0) {
            num_mapper_entries++;
        }
    }

    max_nsb_bits = NEEDED_BITS(settings.max_nsb);

    // create/prepare result bit_stream with number of mapper entries
    *bit_stream = create_bs(num_mapper_entries, max_nsb_bits);

    D(printf("appending num_mapper_entries = %llu (%llu bits)\n", num_mapper_entries, max_nsb_bits);)

    append_num_to_bs(bit_stream, &max_used_nsb, max_nsb_bits);
    D(printf("appending max_used_nsb = %llu (%llu bits)\n", max_used_nsb, max_nsb_bits);)

    max_used_nsb_bits = NEEDED_BITS(max_used_nsb);

    ////////////////////////////////////////////////////////////////////////////////
    // 3. CREATE META DATA (ABOUT THE MAPPER)
    // get max. nsb that has data in the mapper
    // append_num_to_bs(&bs1, &test2, 11);
    // D(printf("max_used_nsb = %llu (needs %d bits)\n", max_used_nsb, max_used_nsb_bits);)

    ////////////////////////////////////////////////////////////////////////////////
    // 4. CREATE MAPPER FROM THE DATA WE'VE COLLECTED
    max_avg_idx_bits = NEEDED_BITS(settings.max_avg_idx);

    // mapper_entry_size = max_used_nsb_bits + 2*max_avg_idx_bits + 1;
    for(nsb = 0; nsb < settings.max_nsb; ++nsb) {
        if(nsb_arrays_lengths[nsb] > 0) {
            // write mapper entry:
            // nsb
            append_num_to_bs(bit_stream, &nsb, max_used_nsb_bits);
            D(printf("appending nsb = %llu (%llu bits)\n", nsb, max_used_nsb_bits);)

            // average
            append_num_to_bs(bit_stream, &nsb_datas[nsb].avg, max_avg_idx_bits);
            D(printf("appending average = %llu (%llu bits)\n", nsb_datas[nsb].avg, max_avg_idx_bits);)

            // max_diff_bits
            // D(printf("appending max_diff = %llu (%llu bits)\n", nsb_datas[nsb].max_diff, max_avg_idx_bits);)
            append_num_to_bs(bit_stream, &nsb_datas[nsb].max_diff_bits, NEEDED_BITS(max_avg_idx_bits));
            D(printf("appending max_diff_bits = %llu (%llu bits)\n", nsb_datas[nsb].max_diff_bits, (number) NEEDED_BITS(max_avg_idx_bits));)
        }
    }

    // mapper is done
    D(printf("mapper size in bits = %llu\n", get_bs_size(bit_stream));)

    ////////////////////////////////////////////////////////////////////////////////
    // CREATE COMPRESSED DATA
    // init nsb_offsets array to 0
    memset(nsb_offsets, 0, settings.max_nsb * sizeof(number));

    // go through nsb_order
    for(i = 0; i < num_blocks; ++i) {
        nsb = nsb_order[i];

        current_nsb_data = nsb_datas[nsb];
        diff = current_nsb_data.diffs[nsb_offsets[nsb]];

        append_num_to_bs(bit_stream, (number *) &diff, current_nsb_data.max_diff_bits);
        // append_num_to_bs(bit_stream, (number *) &diff, NEEDED_BITS(current_nsb_data.max_diff));


        D(printf("appending diff = %lld (%llu bits)\n", diff, current_nsb_data.max_diff_bits);)
        D(printf("...was signed? %lld\n", diff >> (current_nsb_data.max_diff_bits - 1) );)

        // if it's signed
        if(diff >> (current_nsb_data.max_diff_bits - 1)) {
            diff = diff & ((1 << (current_nsb_data.max_diff_bits - 1)) - 1 );
            // write back
            current_nsb_data.diffs[nsb_offsets[nsb]] = -diff;
        }
        D(printf("...original: %lld\n", diff);)

        // increment offset for currently used nsb
        nsb_offsets[nsb]++;
    }
}


bool write_data_to_file(char const filename[], struct bit_stream *bit_stream) {
    number written_bytes;
    unsigned char* output;
    FILE *file;

    written_bytes = 0;

    output = bs_to_byte_stream(bit_stream, &written_bytes);
    D(printf("byte stream has %llu bytes.\n", written_bytes);)

    file = fopen(filename, "wb");
    if(file == NULL)  {
        D(printf("Error: Can't open '%s' for write!\n", filename);)
        return 1;
    }
    fwrite(output, sizeof(unsigned char), written_bytes, file);

    fclose(file);
    return 0;
}
