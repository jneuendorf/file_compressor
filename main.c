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
#ifndef MAX_POSSIBLE_PERM_IDX
// #define MAX_POSSIBLE_PERM_IDX 70
#define MAX_POSSIBLE_PERM_IDX 12870 // binom(16 8)
#endif
#ifndef MAX_AVG_IDX
#define MAX_AVG_IDX (MAX_POSSIBLE_PERM_IDX / 2)
#endif


//////////////////////////////////////////////////////////////////


int main (int argc, char const *argv[]) {
    FILE                *input_file;
    number              **nsb_permutations;
    number              *nsb_order;
    number              *nsb_permutation;
    number              binom_coeff;
    number              block_size;
    number              file_size;
    number              max_used_nsb;
    number              nsb; // number of set bits
    number              nsb_arrays_lengths[MAX_NSB]; //for keeping track of the array sizes
    number              nsb_offsets[MAX_NSB]; //for keeping track of the order of the data blocks
    number              num_blocks;
    number              perm_idx;
    number              permutation;
    struct bit_stream   result;
    struct nsb_data     nsb_datas[MAX_NSB];


    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    // max. 64
    block_size = 16;


    if(argc < 3) {
        D(printf("Usage: %s <input file> <output file>\n", argv[0]);)
        return 1;
    }

    // get command line arguments

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

    num_blocks = CEIL_X_DIV_Y(file_size, (block_size / 8));

    D(printf("file_size = %llu, num_blocks = %llu\n", file_size, num_blocks);)


    // TODO: make this static? (just define huge arrays (without calculations))
    // TODO: this could be done dynamically so unneeded nsb's are not computed
    // 0. fill memory with permutations for each nsb,
    //    also initialize all the pointers of struct nsb_data

    // allocated memory for 1st array dimension
    nsb_permutations = (number **) calloc(MAX_NSB, sizeof(number *));
    // D(printf("allocating %lu bytes\n", sizeof(number *) * MAX_NSB);)

    for(nsb = 0; nsb < MAX_NSB; ++nsb) {
        binom_coeff = binom(MAX_NSB - 1, nsb);
        // D(printf("%d over %llu = %llu\n", MAX_NSB - 1, nsb, binom_coeff);)

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
        } while(next_permutation_bitwise(&permutation, block_size));
        // D(printf("nsb = %llu, permIdx = %llu\n\n", nsb, perm_idx);)
        // D(printf("all permutations loaded for nsb = %llu\n", nsb);)
        // nsb_data pointers
        init_nsb_data(&nsb_datas[nsb], num_blocks);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // 1. READ ENTIRE FILE AND GATHER DATA NEEDED FOR COMPRESSION
    read_data(input_file, nsb_datas, block_size, num_blocks, MAX_NSB, &nsb_order, nsb_arrays_lengths, nsb_permutations);

    fclose(input_file);

    ////////////////////////////////////////////////////////////////////////////////
    // 2. CALCULATE REMAINING DATA FROM READ DATA: AVERAGE, DIFFS
    calc_rem_data(nsb_datas, MAX_NSB, &max_used_nsb, MAX_POSSIBLE_PERM_IDX, nsb_arrays_lengths);

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


    ////////////////////////////////////////////////////////////////////////////////
    // 3. DERIVE REMAINING DATA AND PUT IT INTO A BIT STREAM
    write_data_to_bit_stream(&result, nsb_datas, num_blocks, MAX_NSB, max_used_nsb, MAX_AVG_IDX, nsb_order, nsb_arrays_lengths, nsb_offsets);


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
    // 4. WRITE COMPRESSED DATA TO OUTPUT FILE
    write_data_to_file(argv[2], &result);

    return 0;
}
