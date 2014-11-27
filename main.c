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
    char                *buffer;
    number              block_size;
    number              nsb; // number of set bits
    number              *block;
    number              nsb_permutations[MAX_NSB][MAX_PERM_IDX]; // TODO: improve this: dynamic and smaller sizes (max. perm. index for binom(16 4) is < binom(16 8))
    number              permutation;
    number              perm_idx;
    number              min_perm_idx;
    number              max_perm_idx;
    number              avg_perm_idx;
    struct nsb_data     nsb_datas[MAX_NSB];
    struct nsb_data     current_nsb_data;
    number              block_index;
    number              nsb_arrays_lengths[MAX_NSB]; //for keeping track of the array sizes
    number              nsb_offsets[MAX_NSB]; //for keeping track of the order of the data blocks
    number              *nsb_order;
    // number              max_used_avg_idx;
    number              max_avg_idx_bits;
    number              max_used_nsb;
    unsigned char       max_used_nsb_bits;
    number              diff;
    number              max_diff;
    number              current_nsb;
    unsigned char       negative_diff;
    number              i;
    struct bit_stream   result;

    /*
    printf("endianness = %d\n", is_big_endian());

    // BITSTREAM TEST

    number test = 1199;
    number test2 = 1199;
    struct bit_stream bs1;
    struct bit_stream bs2;
    bs1 = create_bs(test, 11); // 1199 = 0000000000000000000000000000000000000000000000000000010010101111
    // sould equal 10799631906434449408 (11 bits shifted to front and filled up with zeros)
    // 1001010111100000000000000000000000000000000000000000000000000000
    printf("> %llu\n", *(bs1.last_block));
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
    printf("Printing bocks:\n");
    printf(">>> %llu\n", *p);
    while(p != bs1.last_block) {
        printf(">>> %llu\n", *(++p));
    }

    // printf("> avail: %llu\n", bs1.avail_bits); // correct!
    number num_bytes = 0;
    unsigned char *bytes = bs_to_byte_stream(&bs1, &num_bytes);
    unsigned char *p2 = bytes;

    printf("--> %s\n", bits_to_string(bytes, num_bytes));

    for(int i = 0; i < num_bytes; ++i) {
        printf("%d\t->\t%d\t-\t%s\n",i , bytes[i], bits_to_string(p2++, 1));
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
    */

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    // max. 64
    block_size = 16;

    buffer = calloc(block_size / 8 + 1, sizeof(char));

    if(argc < 3) {
        printf("Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }


    // try to open the file
    input_file = fopen(argv[1], "rb");
    if(input_file == NULL)  {
        printf("Error: Can't open '%s' for read!\n", argv[1]);
        return 1;
    }

    // TODO: get file size (for knowing remaining bits and progress)
    // set file position to the end
    fseek(input_file, 0L, SEEK_END);
    // get file size in bytes
    file_size = ftell(input_file);
    // seek back to the beginning
    fseek(input_file, 0L, SEEK_SET);

    num_blocks = CEIL_X_DIV_Y(file_size, (block_size / 8));

    printf("file_size = %llu, num_blocks = %llu\n", file_size, num_blocks);


    // TODO: make this static? (just define huge arrays (without calculations))
    // 0. fill memory with permutations for each nsb (TODO: this could be done dynamically so unneeded nsb's are not computed)
    //    also initialize all the pointers of struct nsb_data
    // TODO: fix next_permutation_bitwise with 0 as argument...seems to be endless
    for(nsb = 1; nsb < MAX_NSB; ++nsb) {
        // printf("nsb = %llu, block_size = %llu...\n", nsb, block_size);
        // permutations
        permutation = (1 << nsb) - 1; // 2^nsb - 1 = nsb many 1s (= the first permutation for that many set bits)
        perm_idx = 0;
        do {
            // printf("current perm_idx = %llu, current permutation = %llu\n", perm_idx, permutation);
            nsb_permutations[nsb][perm_idx++] = permutation;
        } while(next_permutation_bitwise(&permutation, block_size));
        // printf("all permutations loaded for nsb = %llu\n", nsb);
        // nsb_data pointers
        init_nsb_data(&nsb_datas[nsb], num_blocks);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // 1. READ ENTIRE FILE AND GATHER DATA NEEDED FOR COMPRESSION

    // set array to zeros
    // nsb_arrays_lengths = calloc(MAX_NSB, sizeof(number));
    memset(nsb_arrays_lengths, 0, MAX_NSB);
    block_index = 0;

    // p_current_idx = NULL;
    // p_prev_idx = NULL;
    // p_first_idx = NULL;
    nsb_order = calloc(num_blocks, sizeof(number));

    // read file til its end (-> feof = end of file)
    while(!feof(input_file))    {
        // printf("beginning of read loop\n");

        // read next (block_size / 8) bytes (+1 because '\0' is also appended)
        fgets(buffer, block_size / 8 + 1, input_file);

        printf("buffer: %s (%lu)\n", buffer, strlen(buffer));

        block = (number *) buffer;
        // printf("%d\n", *block);

        current_nsb = (number) number_of_set_bits(*block); // TODO: for now, only works for 32 bit integers

        // printf("current nsb = %llu in %llu = %s\n", current_nsb, *((number *) buffer), bits_to_string(buffer, block_size));
        // printf("current nsb = %llu @ %llu\n", current_nsb, block_index);

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
        block_index++; // block done
    }
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

        // // save maxi
        // if(avg_perm_idx > max_used_avg_idx) {
        //
        // }

        nsb_datas[nsb].avg = avg_perm_idx;

        // get differences of perm. indices to the average perm. index (perm_idx - avg_perm_idx)
        max_diff = 0;
        for(i = 0; i < nsb_arrays_lengths[nsb]; ++i) {
            diff = nsb_datas[nsb].perm_indices[i] - avg_perm_idx;
            nsb_datas[nsb].diffs[i] = diff;
            if((diff = abs(diff)) > max_diff) {
                max_diff = diff;
            }
        }
        // nsb_datas[nsb].max_diff_bits = (unsigned char) NEEDED_BITS(max_diff);
        nsb_datas[nsb].max_diff = max_diff;
    }

    // // test: print indices and perm. indices for each nsb
    // for(nsb = 1; nsb < MAX_NSB; ++nsb) {
    //     // indices
    //     printf("data for %llu:\n\t.indices = ", nsb);
    //
    //     for(i = 0; i < nsb_arrays_lengths[nsb]; i++) {
    //         printf("%llu, ", nsb_datas[nsb].indices[i]);
    //     }
    //     printf("\n");
    //
    //     // permutation indices
    //     printf("\t.perm_indices = ");
    //
    //     for(i = 0; i < nsb_arrays_lengths[nsb]; i++) {
    //         printf("%llu, ", nsb_datas[nsb].perm_indices[i]);
    //     }
    //     printf("\n");
    //
    //     // average
    //     printf("\t.avg = %llu\n", nsb_datas[nsb].avg);
    //
    //     // diffs
    //     printf("\t.diffs = ");
    //
    //     for(i = 0; i < nsb_arrays_lengths[nsb]; i++) {
    //         printf("%lld, ", nsb_datas[nsb].diffs[i]);
    //     }
    //     printf("\n");
    //
    //     // max diff
    //     printf("\t.max_diff = %llu\n", nsb_datas[nsb].max_diff);
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
    printf("appending num_mapper_entries = %llu (%llu bits)\n", num_mapper_entries, (number) NEEDED_BITS(MAX_NSB));



    // result = create_bs(max_used_nsb, NEEDED_BITS(MAX_NSB));
    append_num_to_bs(&result, &max_used_nsb, NEEDED_BITS(MAX_NSB));
    printf("appending max_nsb = %llu (%llu bits)\n", max_used_nsb, (number) NEEDED_BITS(MAX_NSB));

    max_used_nsb_bits = NEEDED_BITS(max_used_nsb); // floor(log2(max_used_nsb)) + 1;

    ////////////////////////////////////////////////////////////////////////////////
    // 3. CREATE META DATA (ABOUT THE MAPPER)
    // get max. nsb that has data in the mapper
    // append_num_to_bs(&bs1, &test2, 11);
    printf("max_used_nsb = %llu (needs %d bits)\n", max_used_nsb, max_used_nsb_bits);

    ////////////////////////////////////////////////////////////////////////////////
    // 4. CREATE MAPPER FROM THE DATA WE'VE COLLECTED
    max_avg_idx_bits = NEEDED_BITS(MAX_AVG_IDX); //floor(log2(MAX_AVG_IDX)) + 1; //

    // mapper_entry_size = max_used_nsb_bits + 2*max_avg_idx_bits + 1;
    for(nsb = 1; nsb < MAX_NSB; ++nsb) {
        if(nsb_arrays_lengths[nsb] > 0) {
            // write mapper entry:
            // nsb
            append_num_to_bs(&result, &nsb, max_used_nsb_bits);
            printf("appending nsb = %llu (%hhu bits)\n", nsb, max_used_nsb_bits);
            // average
            append_num_to_bs(&result, &nsb_datas[nsb].avg, max_avg_idx_bits);
            printf("appending average = %llu (%llu bits)\n", nsb_datas[nsb].avg, max_avg_idx_bits);
            // negavtive_diffs
            negative_diff = 0;
            for(i = 0; i < nsb_arrays_lengths[nsb]; ++i) {
                if(nsb_datas[nsb].diffs[i] < 0) {
                    negative_diff = 1;
                    break;
                }
            }
            append_num_to_bs(&result, (number *) &negative_diff, 1);
            printf("appending neg_diff_flag = %d (1 bit)\n", negative_diff);
            // max_diff
            append_num_to_bs(&result, (number *) &nsb_datas[nsb].max_diff, max_avg_idx_bits);
            printf("appending max_diff = %llu (%llu bits)\n", nsb_datas[nsb].max_diff, max_avg_idx_bits);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // 5. CREATE COMPRESSED DATA
    // init nsb_offsets array to 0
    memset(nsb_offsets, 0, MAX_NSB * sizeof(number)); // or init it like number nsb_offsets[MAX_NSB] = {0}; at beginning of main()
    // go through nsb_order
    for(i = 0; i < num_blocks; ++i) {
        nsb = nsb_order[i];
        printf("appending diff = %lld\n", nsb_datas[nsb].diffs[nsb_offsets[nsb]]);
        append_num_to_bs(&result, (number *) &nsb_datas[nsb].diffs[nsb_offsets[nsb]], NEEDED_BITS(nsb_datas[nsb].max_diff));
    }

    number *p = result.bits;
    printf("Printing bocks:\n");
    printf(">>> %llu\n", *p);
    while(p != result.last_block) {
        printf(">>> %llu\n", *(++p));
    }
    printf("last block available = %d\n", result.avail_bits);
    // prints:
    // 1270162979246268160 = 0001000110100000100001101000000000000000110100110101111100000000
    // 0                   = 00000000
    // last block available = 56
    // this makes:
    // 000100011010000010000110100000000000000011010011010111110000000000000000
    //
    // expected:
    // 00010|00110|100 0000000011010 0 0000000000000|110 1000101011111 0 0000000000000|0 0
    // 00010 00110 100 0001000011010 0 0000000000000 110 1001101011111 0 0000000000000 0 0
    // correct!

    ////////////////////////////////////////////////////////////////////////////////
    // 6. WRITE COMPRESSED DATA TO OUTPUT FILE
    number written_bytes = 0;
    unsigned char* output = bs_to_byte_stream(&result, &written_bytes);
    printf("byte stream has %llu bytes\nprinting...\n", written_bytes);
    for(i = 0; i < written_bytes; ++i) {
        printf("%d\n", (unsigned char) output[i]);
    }
    // prints: (from 00010001 10100000 10000110 10000000 00000000 11010011 01011111 00000000 00000000)
    // 17   correct
    // 160  correct
    // 134  correct
    // 128  correct
    // 0    correct
    // 211  correct
    // 95   correct
    // 0    correct
    // 0    correct

    output_file = fopen(argv[2], "wb");
    if(output_file == NULL)  {
        printf("Error: Can't open '%s' for write!\n", argv[2]);
        return 1;
    }
    fwrite(output, sizeof(unsigned char), written_bytes, output_file);

    return 0;
}
