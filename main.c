#include "global.h"
#include "permutation.h"
#include "compressor.h"
#include "compress.h"
#include "decompress.h"

//////////////////////////////////////////////////////////////////
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 16
#endif
// TODO: make this dynamic!
#ifndef MAX_NSB
#define MAX_NSB (BLOCK_SIZE + 1)
#endif
#ifndef MAX_PERM_IDX
// #define MAX_PERM_IDX 70
#define MAX_PERM_IDX 12870 // binom(16 8)
#endif
#ifndef MAX_AVG_IDX
#define MAX_AVG_IDX (MAX_PERM_IDX / 2)
#endif


//////////////////////////////////////////////////////////////////


int main (int argc, char const *argv[]) {
    FILE                *input_file;
    FILE                *output_file;
    number              file_size;
    number              num_blocks;
    number              num_mapper_entries;
    // char                *buffer;
    number              block_size;
    number              nsb; // number of set bits
    number              **nsb_permutations;
    number              *nsb_permutation;
    number              permutation;
    number              perm_idx;
    number              min_perm_idx;
    number              max_perm_idx;
    number              avg_perm_idx;
    struct nsb_data     nsb_datas[MAX_NSB];
    // struct nsb_data     current_nsb_data;
    // number              block_index;
    number              nsb_arrays_lengths[MAX_NSB]; //for keeping track of the array sizes
    number              nsb_offsets[MAX_NSB]; //for keeping track of the order of the data blocks
    number              *nsb_order;
    // number              max_used_avg_idx;
    number              max_avg_idx_bits;
    number              max_used_nsb;
    unsigned char       max_used_nsb_bits;
    signed_number       diff;
    number              max_diff_bits;
    // number              current_nsb;
    // number              negative_diff;
    number              i;
    struct bit_stream   result;
    number              binomCoeff;


    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    // max. 64
    block_size = 16;


    if(argc < 3) {
        D(printf("Usage: %s <input file> <output file>\n", argv[0]);)
        return 1;
    }

    // try to open the file
    input_file = fopen(argv[1], "rb");
    if(input_file == NULL)  {
        D(printf("Error: Can't open '%s' for read!\n", argv[1]);)
        return 1;
    }

    // TODO: show progress
    // set file position to the end
    fseek(input_file, 0L, SEEK_END);
    // get file size in bytes
    file_size = ftell(input_file);
    // seek back to the beginning
    fseek(input_file, 0L, SEEK_SET);

    num_blocks = CEIL_X_DIV_Y(file_size, (block_size / 8));

    D(printf("file_size = %llu, num_blocks = %llu\n", file_size, num_blocks);)


    // TODO: make this static? (just define huge arrays (without calculations))
    // TODO: this could be done dynamically so unneeded nsb's are not computed
    // 0. fill memory with permutations for each nsb,
    //    also initialize all the pointers of struct nsb_data

    // allocated memory for 1st array dimension
    nsb_permutations = (number **) calloc(MAX_NSB, sizeof(number *));
    D(printf("allocating %lu bytes\n", sizeof(number *) * MAX_NSB);)

    for(nsb = 0; nsb < MAX_NSB; ++nsb) {
        binomCoeff = binom(MAX_NSB - 1, nsb);
        // D(printf("%d over %llu = %llu\n", MAX_NSB - 1, nsb, binomCoeff);)

        // allocate needed memory
        nsb_permutation = (number *) calloc(binomCoeff, sizeof(number));
        // D(printf("allocating %llu bytes\n", sizeof(number *) * binomCoeff);)
        nsb_permutations[nsb] = nsb_permutation; // or (*nsb_permutations + nsb)

        // permutations
        permutation = (1 << nsb) - 1; // 2^nsb - 1 = nsb many 1s (= the first permutation for that many set bits)
        perm_idx = 0;
        do {
            // D(printf("current perm_idx = %llu, current permutation = %llu\n", perm_idx, permutation);)
            nsb_permutation[perm_idx++] = permutation;
        } while(next_permutation_bitwise(&permutation, block_size));
        // D(printf("nsb = %llu, permIdx = %llu\n\n", nsb, perm_idx);)
        // D(printf("all permutations loaded for nsb = %llu\n", nsb);)
        // nsb_data pointers
        init_nsb_data(&nsb_datas[nsb], num_blocks);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // 1. READ ENTIRE FILE AND GATHER DATA NEEDED FOR COMPRESSION

    read_data(input_file, nsb_datas, block_size, num_blocks, MAX_NSB, &nsb_order, nsb_arrays_lengths, nsb_permutations);

    // buffer = calloc(block_size / 8 + 1, sizeof(char));
    //
    // // set array 'nsb_arrays_lengths' to zeros
    // memset(nsb_arrays_lengths, 0, MAX_NSB * sizeof(number));
    // block_index = 0;
    //
    // nsb_order = calloc(num_blocks, sizeof(number));
    //
    // // read file til its end (-> feof = end of file)
    // while(!feof(input_file))    {
    //     // D(printf("beginning of read loop\n");)
    //
    //     // read next (block_size / 8) bytes (+1 because '\0' is also appended)
    //     // fgets(buffer, block_size / 8 + 1, input_file);
    //     fread(buffer, block_size / 8, 1, input_file);
    //
    //     // D(printf("buffer: %s (%lu)\n", buffer, strlen(buffer));)
    //     D(printf("buffer: %x,%x (%lu bytes)\n", (unsigned short) *buffer, (unsigned short) buffer[1], block_size / 8);)
    //
    //     block = (number *) buffer;
    //     // D(printf("%d\n", *block);)
    //
    //     current_nsb = (number) number_of_set_bits(*block); // TODO: for now, only works for 32 bit integers
    //
    //     // D(printf("current nsb = %llu in %llu = %s\n", current_nsb, *((number *) buffer), bits_to_string(buffer, block_size));)
    //     // D(printf("current nsb = %llu @ %llu\n", current_nsb, block_index);)
    //
    //     current_nsb_data = nsb_datas[current_nsb];
    //
    //     current_nsb_data.indices[nsb_arrays_lengths[current_nsb]] = block_index;
    //
    //     // keep track of the order of blocks in our data structure
    //     nsb_order[block_index] = current_nsb;
    //
    //     // get permutation index of current block
    //     perm_idx = 0;
    //     while(nsb_permutations[current_nsb][perm_idx++] != *block) {}
    //
    //     current_nsb_data.perm_indices[nsb_arrays_lengths[current_nsb]] = perm_idx;
    //
    //     // diffs can be calculated after the average has been determined (after the whole file has been read!)
    //     nsb_arrays_lengths[current_nsb]++; // an element was pushed
    //     block_index++; // block done
    // }
    fclose(input_file);

    ////////////////////////////////////////////////////////////////////////////////
    // 2. CALCULATE REMAINING DATA FROM READ DATA: AVERAGE, DIFFS
    max_used_nsb = 0;
    // max_used_avg_idx = 0;
    for(nsb = 1; nsb < MAX_NSB; ++nsb) {
        // get minimal and maximal permutation indices and maximal nsb with data
        // there is data for that nsb
        if(nsb_arrays_lengths[nsb] > 0) {
            // current nsb is the current max. nsb
            max_used_nsb = nsb;

            // get min and max
            min_perm_idx = MAX_PERM_IDX;
            max_perm_idx = 0;
            for(i = 0; i < nsb_arrays_lengths[nsb]; ++i) {
                perm_idx = nsb_datas[nsb].perm_indices[i];
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
        for(i = 0; i < nsb_arrays_lengths[nsb]; ++i) {
            diff = nsb_datas[nsb].perm_indices[i] - avg_perm_idx;
            nsb_datas[nsb].diffs[i] = diff;

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
        for(i = 0; i < nsb_arrays_lengths[nsb]; ++i) {
            diff = nsb_datas[nsb].diffs[i];
            // if diff is negative take care of creating a bit pattern we interpret as negative because we don't wanna use 2's complement because this needs as many bits as the data type it is saved in
            // so in that case we add '1' to the front (most significant bit)
            if(diff < 0) {
                // make positive
                diff = -diff;
                // OR it with a number that has
                diff = diff | (1 << (max_diff_bits - 1));
                // write back to data
                nsb_datas[nsb].diffs[i] = diff;
            }
        }
        nsb_datas[nsb].max_diff_bits = max_diff_bits;
    }

    // test: print indices and perm. indices for each nsb
    // for(nsb = 1; nsb < MAX_NSB; ++nsb) {
    //     // indices
    //     D(printf("data for %llu:\n\t.indices = ", nsb);)
    //
    //     for(i = 0; i < nsb_arrays_lengths[nsb]; i++) {
    //         D(printf("%llu, ", nsb_datas[nsb].indices[i]);)
    //     }
    //     D(printf("\n");)
    //
    //     // permutation indices
    //     D(printf("\t.perm_indices = ");)
    //
    //     for(i = 0; i < nsb_arrays_lengths[nsb]; i++) {
    //         D(printf("%llu, ", nsb_datas[nsb].perm_indices[i]);)
    //     }
    //     D(printf("\n");)
    //
    //     // average
    //     D(printf("\t.avg = %llu\n", nsb_datas[nsb].avg);)
    //
    //     // diffs
    //     D(printf("\t.diffs = ");)
    //
    //     for(i = 0; i < nsb_arrays_lengths[nsb]; i++) {
    //         D(printf("%lld, ", nsb_datas[nsb].diffs[i]);)
    //     }
    //     D(printf("\n");)
    //
    //     // max diff
    //     // D(printf("\t.max_diff = %llu\n", nsb_datas[nsb].max_diff);)
    //     D(printf("\t.max_diff_bits = %llu\n", nsb_datas[nsb].max_diff_bits);)
    // }

    // create/prepare result bit_stream with number of mapper entries
    // get number of mapper entries
    num_mapper_entries = 0;
    for(nsb = 1; nsb < MAX_NSB; ++nsb) {
        if(nsb_arrays_lengths[nsb] > 0) {
            num_mapper_entries++;
        }
    }
    result = create_bs(num_mapper_entries, NEEDED_BITS(MAX_NSB));
    D(printf("appending num_mapper_entries = %llu (%llu bits)\n", num_mapper_entries, (number) NEEDED_BITS(MAX_NSB));)

    // result = create_bs(max_used_nsb, NEEDED_BITS(MAX_NSB));
    append_num_to_bs(&result, &max_used_nsb, NEEDED_BITS(MAX_NSB));
    D(printf("appending max_used_nsb = %llu (%llu bits)\n", max_used_nsb, (number) NEEDED_BITS(MAX_NSB));)

    max_used_nsb_bits = NEEDED_BITS(max_used_nsb); // floor(log2(max_used_nsb)) + 1;

    ////////////////////////////////////////////////////////////////////////////////
    // 3. CREATE META DATA (ABOUT THE MAPPER)
    // get max. nsb that has data in the mapper
    // append_num_to_bs(&bs1, &test2, 11);
    D(printf("max_used_nsb = %llu (needs %d bits)\n", max_used_nsb, max_used_nsb_bits);)

    ////////////////////////////////////////////////////////////////////////////////
    // 4. CREATE MAPPER FROM THE DATA WE'VE COLLECTED
    max_avg_idx_bits = NEEDED_BITS(MAX_AVG_IDX); //floor(log2(MAX_AVG_IDX)) + 1; //

    // mapper_entry_size = max_used_nsb_bits + 2*max_avg_idx_bits + 1;
    for(nsb = 1; nsb < MAX_NSB; ++nsb) {
        if(nsb_arrays_lengths[nsb] > 0) {
            // write mapper entry:
            // nsb
            append_num_to_bs(&result, &nsb, max_used_nsb_bits);
            D(printf("appending nsb = %llu (%hhu bits)\n", nsb, max_used_nsb_bits);)

            // average
            append_num_to_bs(&result, &nsb_datas[nsb].avg, max_avg_idx_bits);
            D(printf("appending average = %llu (%llu bits)\n", nsb_datas[nsb].avg, max_avg_idx_bits);)

            // max_diff_bits
            // append_num_to_bs(&result, (number *) &nsb_datas[nsb].max_diff, max_avg_idx_bits);
            // D(printf("appending max_diff = %llu (%llu bits)\n", nsb_datas[nsb].max_diff, max_avg_idx_bits);)
            append_num_to_bs(&result, &nsb_datas[nsb].max_diff_bits, NEEDED_BITS(max_avg_idx_bits));
            D(printf("appending max_diff_bits = %llu (%llu bits)\n", nsb_datas[nsb].max_diff_bits, (number) NEEDED_BITS(max_avg_idx_bits));)
            D(printf(">>> %llu\n", *(result.bits));)
            // 00010 00110 100 0000000011010 0 0001 000000000000000000000000000000000
            // 00010 00110 100 0000000011010 0 0001
        }
    }

    // mapper is done
    D(printf("mapper size in bits = %llu\n", get_bs_size(&result));)

    ////////////////////////////////////////////////////////////////////////////////
    // 5. CREATE COMPRESSED DATA
    // init nsb_offsets array to 0
    memset(nsb_offsets, 0, MAX_NSB * sizeof(number)); // or init it like number nsb_offsets[MAX_NSB] = {0}; at beginning of main()
    // go through nsb_order
    for(i = 0; i < num_blocks; ++i) {
        nsb = nsb_order[i];

        append_num_to_bs(&result, (number *) &nsb_datas[nsb].diffs[nsb_offsets[nsb]], nsb_datas[nsb].max_diff_bits);
        // append_num_to_bs(&result, (number *) &nsb_datas[nsb].diffs[nsb_offsets[nsb]], NEEDED_BITS(nsb_datas[nsb].max_diff));

        number diff = nsb_datas[nsb].diffs[nsb_offsets[nsb]];

        D(printf("appending diff = %lld (%llu bits)\n", diff, nsb_datas[nsb].max_diff_bits);)
        D(printf("...was signed? %lld\n", ((number) nsb_datas[nsb].diffs[nsb_offsets[nsb]]) >> (nsb_datas[nsb].max_diff_bits - 1) );)

        if(nsb_datas[nsb].diffs[nsb_offsets[nsb]] >> (nsb_datas[nsb].max_diff_bits - 1)) {
            nsb_datas[nsb].diffs[nsb_offsets[nsb]] = nsb_datas[nsb].diffs[nsb_offsets[nsb]] & ((1 << (nsb_datas[nsb].max_diff_bits - 1)) - 1 );
            nsb_datas[nsb].diffs[nsb_offsets[nsb]] = -nsb_datas[nsb].diffs[nsb_offsets[nsb]];
        }
        D(printf("...original: %lld\n", nsb_datas[nsb].diffs[nsb_offsets[nsb]]);)

        // increment offset for currently used nsb
        nsb_offsets[nsb]++;
    }

    number *p = result.bits;
    D(printf("Printing blocks:\n");)
    D(printf("1st block = %p, last_block = %p\n", result.bits, result.last_block);)
    D(printf(">>> %llu\n", *p);)
    while(p != result.last_block) {
        D(printf(">>> %llu\n", *(++p));)
        // D(printf(">>> %llu < %llu\n", p, result.last_block);)
    }
    D(printf("last block available = %d\n", result.avail_bits);)


    ////////////////////////////////////////////////////////////////////////////////
    // 6. WRITE COMPRESSED DATA TO OUTPUT FILE
    number written_bytes = 0;
    unsigned char* output = bs_to_byte_stream(&result, &written_bytes);
    D(printf("byte stream has %llu bytes.\n", written_bytes);)

    output_file = fopen(argv[2], "wb");
    if(output_file == NULL)  {
        D(printf("Error: Can't open '%s' for write!\n", argv[2]);)
        return 1;
    }
    fwrite(output, sizeof(unsigned char), written_bytes, output_file);

    return 0;
}
