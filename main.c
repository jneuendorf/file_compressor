#include "global.h"
#include "permutation.h"
#include "compressor.h"
#include "compress.h"
#include "decompress.h"


//////////////////////////////////////////////////////////////////
// MAIN
int main (int argc, char const *argv[]) {
    FILE                *input_file;
    number              **nsb_permutations;
    number              *nsb_order;
    number              *nsb_permutation;
    number              binom_coeff;
    number              file_size;
    number              max_used_nsb;
    number              nsb; // number of set bits
    // number              nsb_arrays_lengths[MAX_NSB]; //for keeping track of the array sizes
    number              *nsb_arrays_lengths; //for keeping track of the array sizes
    // number              nsb_offsets[MAX_NSB]; //for keeping track of the order of the data blocks
    number              *nsb_offsets; //for keeping track of the order of the data blocks
    number              num_blocks;
    number              perm_idx;
    number              permutation;
    struct bit_stream   result;
    // struct nsb_data     nsb_datas[MAX_NSB];
    struct nsb_data     *nsb_datas;
    unsigned char       block_size;


    ////////////////////////////////////////////////////////////////////////////////
    // COMMAND LINE ARGUMENTS
    if(argc < 3) {
        D(printf("Usage: %s <input file> <output file>\n", argv[0]);)
        return 1;
    }

    // init options
    settings.compress = true;
    settings.verbose = false;
    settings.block_size = 16;
    // get command line arguments
    if(argc > 3) {
        for(unsigned char i = 3; i < argc; ++i) {
            if(strcmp("-v", argv[i]) == 0 || strcmp("-v", argv[i]) == 0) {
                settings.verbose = true;
            }
            else if(strcmp("--decompress", argv[i]) == 0) {
                settings.compress = false;
            }
            else if(strcmp("-b", argv[i]) == 0) {
                block_size = strtol(argv[++i], NULL, 10);
                // set only if valid
                if(block_size >= 8 && block_size <= 64 && block_size % 8 == 0) {
                    settings.block_size = block_size;
                }
            }
        }
    }
    // set greatest possible NSB (block_size + 1 because from 0 to block_size)
    settings.max_nsb = settings.block_size + 1;
    // set greatest possible permutation index
    settings.max_perm_idx = binom(settings.block_size, settings.block_size / 2);
    // set greatest possible average permutation index (half of max. permutation index)
    settings.max_avg_idx = settings.max_perm_idx / 2;
    D(printf("settings: verbose = %d, compress = %d, block_size = %u\n", settings.verbose, settings.compress, settings.block_size);)

    // try to open the file
    input_file = fopen(argv[1], "rb");
    if(input_file == NULL)  {
        printf("Error: Can't open '%s' for read!\n", argv[1]);
        return 1;
    }

    // TODO: show progress
    // set file position to the end
    fseek(input_file, 0L, SEEK_END);
    // get file size in bytes
    file_size = ftell(input_file);
    // seek back to the beginning
    fseek(input_file, 0L, SEEK_SET);

    num_blocks = CEIL_X_DIV_Y(file_size, (settings.block_size / 8));

    D(printf("file_size = %llu bytes, num_blocks = %llu\n", file_size, num_blocks);)


    // allocated memory for dynamic arrays
    nsb_permutations    = (number **) calloc(settings.max_nsb, sizeof(number *));
    nsb_arrays_lengths  = (number *)  calloc(settings.max_nsb, sizeof(number));
    nsb_offsets         = (number *)  calloc(settings.max_nsb, sizeof(number));
    nsb_datas           = (struct nsb_data *) calloc(settings.max_nsb, sizeof(struct nsb_data));


    // TODO: make this static? (just define huge arrays (without calculations))
    // TODO: this could be done dynamically so unneeded nsb's are not computed
    // 0. fill memory with permutations for each nsb,
    //    also initialize all the pointers of struct nsb_data
    for(nsb = 0; nsb < settings.max_nsb; ++nsb) {
        binom_coeff = binom(settings.max_nsb - 1, nsb);
        // D(printf("%d over %llu = %llu\n", settings.max_nsb - 1, nsb, binom_coeff);)

        // allocate needed memory
        nsb_permutation = (number *) calloc(binom_coeff, sizeof(number));
        // D(printf("allocating %llu bytes\n", sizeof(number *) * binom_coeff);)
        nsb_permutations[nsb] = nsb_permutation; // or (*nsb_permutations + nsb)

        // permutations
        permutation = (1 << nsb) - 1; // 2^nsb - 1 = nsb many 1s (= the first permutation for that many set bits)
        perm_idx = 0;
        do {
            // D(printf("current perm_idx = %llu, current permutation = %llu\n", perm_idx, permutation);)
            nsb_permutation[perm_idx++] = permutation;
        } while(next_permutation_bitwise(&permutation, settings.block_size));
        // D(printf("nsb = %llu, permIdx = %llu\n\n", nsb, perm_idx);)
        // D(printf("all permutations loaded for nsb = %llu\n", nsb);)
        // nsb_data pointers
        init_nsb_data(&nsb_datas[nsb], num_blocks);
    }

    if(settings.verbose) {
        printf("all permutations loaded...\n");
        printf("%llu blocks to compress...\n", num_blocks);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // 1. READ ENTIRE FILE AND GATHER DATA NEEDED FOR COMPRESSION
    if(settings.verbose) {
        printf("reading %s...\n", argv[1]);
    }

    read_data(input_file, nsb_datas, num_blocks, &nsb_order, nsb_arrays_lengths, nsb_permutations);
    fclose(input_file);

    if(settings.verbose) {
        printf("done reading %s...\n", argv[1]);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // 2. CALCULATE REMAINING DATA FROM READ DATA: AVERAGE, DIFFS
    if(settings.verbose) {
        printf("calculating data for compression...\n");
    }

    calc_rem_data(nsb_datas, &max_used_nsb, nsb_arrays_lengths);

    if(settings.verbose) {
        printf("done calculating data for compression...\n");
    }

    // test: print indices and perm. indices for each nsb
    // for(nsb = 1; nsb < settings.max_nsb; ++nsb) {
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


    ////////////////////////////////////////////////////////////////////////////////
    // 3. DERIVE REMAINING DATA AND PUT IT INTO A BIT STREAM
    if(settings.verbose) {
        printf("creating bit stream that'l be written...\n");
    }

    write_data_to_bit_stream(&result, nsb_datas, num_blocks, max_used_nsb, nsb_order, nsb_arrays_lengths, nsb_offsets);

    if(settings.verbose) {
        printf("bit stream is ready...\n");
    }


    // number *p = result.bits;
    // D(printf("Printing blocks:\n");)
    // D(printf("1st block = %p, last_block = %p\n", result.bits, result.last_block);)
    // D(printf(">>> %llu\n", *p);)
    // while(p != result.last_block) {
    //     D(printf(">>> %llu\n", *(++p));)
    //     // D(printf(">>> %llu < %llu\n", p, result.last_block);)
    // }
    // D(printf("last block available = %d\n", result.avail_bits);)


    ////////////////////////////////////////////////////////////////////////////////
    // 4. WRITE COMPRESSED DATA TO OUTPUT FILE
    if(settings.verbose) {
        printf("writing bit stream to %s...\n", argv[2]);
    }

    write_data_to_file(argv[2], &result);

    if(settings.verbose) {
        printf("DONE!\n");
    }

    return 0;
}
