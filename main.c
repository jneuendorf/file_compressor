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
    number              *nsb_arrays_lengths; //for keeping track of the array sizes
    number              *nsb_offsets; //for keeping track of the order of the data blocks
    number              *nsb_order;
    number              *nsb_permutation;
    number              binom_coeff;
    number              file_size;
    number              max_used_nsb;
    number              nsb; // number of set bits
    number              perm_idx;
    number              permutation;
    struct bit_stream   bit_stream;
    struct nsb_data     *nsb_datas;


    // number test = 31; // 1111 1111
    // do {
    //     // D(printf("current perm_idx = %llu, current permutation = %llu\n", perm_idx, permutation);)
    //     printf("%s\n", bits_to_string(&test, 8));
    // } while(next_permutation_bitwise(&test, 8));
    // return 0;


    ////////////////////////////////////////////////////////////////////////////////
    // COMMAND LINE ARGUMENTS
    if(argc < 3) {
        D(printf("Usage: %s <input file> <output file>\n", argv[0]);)
        return 1;
    }

    // init options from command line arguments
    parse_cmd_line_arguments(argc, argv, &settings);

    D(printf("settings => verbose = %d, compress = %d, block_size = %u\n", settings.verbose, settings.compress, settings.block_size);)

    // COMPRESSION
    if(settings.compress) {
        // try to open the file
        input_file = fopen(argv[1], "rb");
        if(input_file == NULL)  {
            printf("Error: Can't open '%s' for read!\n", argv[1]);
            return 1;
        }

        file_size = get_file_size(input_file);

        // fill compress_data struct
        compress_data.block_size = settings.block_size;
        compress_data.num_blocks = CEIL_X_DIV_Y(file_size, (compress_data.block_size / 8));
        // set greatest possible NSB (block_size + 1 because from 0 to block_size)
        compress_data.max_nsb = compress_data.block_size + 1;
        // set greatest possible permutation index
        compress_data.max_perm_idx = binom(compress_data.block_size, compress_data.block_size / 2);
        // set greatest possible average permutation index (half of max. permutation index)
        compress_data.max_avg_idx = compress_data.max_perm_idx / 2;

        D(printf("file_size = %llu bytes, num_blocks = %llu\n", file_size, compress_data.num_blocks);)

        // allocated memory for dynamic arrays
        nsb_permutations    = (number **) calloc(compress_data.max_nsb, sizeof(number *));
        nsb_arrays_lengths  = (number *)  calloc(compress_data.max_nsb, sizeof(number));
        nsb_offsets         = (number *)  calloc(compress_data.max_nsb, sizeof(number));
        nsb_datas           = (struct nsb_data *) calloc(compress_data.max_nsb, sizeof(struct nsb_data));


        // TODO: make this static? (just define huge arrays (without calculations))
        // TODO: this could be done dynamically so unneeded nsb's are not computed
        // 0. fill memory with permutations for each nsb,
        //    also initialize all the pointers of struct nsb_data
        for(nsb = 0; nsb < compress_data.max_nsb; ++nsb) {
            binom_coeff = binom(compress_data.max_nsb - 1, nsb);
            // D(printf("%d over %llu = %llu\n", compress_data.max_nsb - 1, nsb, binom_coeff);)

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
            } while(next_permutation_bitwise(&permutation, compress_data.block_size));
            // D(printf("nsb = %llu, permIdx = %llu\n\n", nsb, perm_idx);)
            // D(printf("all permutations loaded for nsb = %llu\n", nsb);)
            // nsb_data pointers
            init_nsb_data(&nsb_datas[nsb]);
        }

        if(settings.verbose) {
            printf("all permutations loaded...\n");
            printf("%llu blocks to compress...\n", compress_data.num_blocks);
        }

        ////////////////////////////////////////////////////////////////////////////////
        // 1. READ ENTIRE FILE AND GATHER DATA NEEDED FOR COMPRESSION
        if(settings.verbose) {
            printf("reading %s...\n", argv[1]);
        }

        read_uncompressed_data(input_file, nsb_datas, &nsb_order, nsb_arrays_lengths, nsb_permutations);
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

        // TODO: before writing data to bit stream permute blocks of compressed data (see globa.h@max_data_perm_block_size)
        // TODO: try to get 1 block of each NSB...if not prepend NSBs that are not in the compressed data blocks (like an ignore list)
        // TODO: otherwise use 1 bit for indicating whether to take the same NSB for the next block or the next NSB in the mapper
        // TODO: (after the whole block is done - restart)

        write_data_to_bit_stream(&bit_stream, nsb_datas, max_used_nsb, nsb_order, nsb_arrays_lengths, nsb_offsets);

        if(settings.verbose) {
            printf("bit stream is ready...\n");
        }

        number *p = bit_stream.bits;
        D(printf("Printing blocks:\n");)
        D(printf("1st block = %p, last_block = %p\n", bit_stream.bits, bit_stream.last_block);)
        D(printf(">>> %llu\n", *p);)
        while(p != bit_stream.last_block) {
            D(printf(">>> %llu\n", *(++p));)
            // D(printf(">>> %llu < %llu\n", p, bit_stream.last_block);)
        }
        D(printf("last block available = %d\n", bit_stream.avail_bits);)


        ////////////////////////////////////////////////////////////////////////////////
        // 4. WRITE COMPRESSED DATA TO OUTPUT FILE
        if(settings.verbose) {
            printf("writing bit stream to %s...\n", argv[2]);
        }

        write_data_to_file(argv[2], &bit_stream);

        if(settings.verbose) {
            printf("DONE!\n");
        }
    }
    // DECOMPRESSION
    else {
        // // little read_bs() test
        // unsigned char read_error = 0;
        // // 16450297403121329451 = 11100100010010 110011001001010100101010 10100100111110010100101011
        // number n = 16450297403121329451UL;
        // // 2356 = 100100110100
        // number m = 2356UL;
        // number read = 0;
        // struct bit_stream test = create_bs(n, 64);
        // append_num_to_bs(&test, &m, 12);
        //
        // read = read_bs(&test, 14, &read_error);
        // D(printf("1. read block = %llu (%u)\n", read, read_error);)
        // // expected 11100100010010 = 14610
        //
        // read = read_bs(&test, 24, &read_error);
        // D(printf("2. read block = %llu (%u)\n", read, read_error);)
        // // expected 110011001001010100101010 = 13407530
        //
        // read = read_bs(&test, 38, &read_error);
        // D(printf("3. read block = %llu (%u)\n", read, read_error);)
        // // expected 10100100111110010100101011100100110100 = 177139267892
        //
        // return 0;

        // read data from compressed file to bit stream (block-wise)
        read_compressed_data(argv[1], &bit_stream);



        D(printf("decompress_data => block_size = %u, max_nsb = %u, num_mapper_entries = %u, max_used_nsb = %u\n", decompress_data.block_size, decompress_data.max_nsb, decompress_data.num_mapper_entries, decompress_data.max_used_nsb);)


        number *p = bit_stream.bits;
        D(printf("Printing blocks:\n");)
        // D(printf("1st block = %p, last_block = %p\n", bit_stream.bits, bit_stream.last_block);)
        D(printf(">>> %llu\n", *p);)
        while(p != bit_stream.last_block) {
            D(printf(">>> %llu\n", *(++p));)
            // D(printf(">>> %llu < %llu\n", p, bit_stream.last_block);)
        }
        D(printf("last block available = %d\n", bit_stream.avail_bits);)
        // expected: 2467562449949822976 = 001 00010 001111 10100010101111100101111010010110100001000000000000
        // prints:   2467562449949822976 = 001 0001000111110100010101111100101111010010110100001000000000000
        // nsb = [7,9,9,1]
    }


    return 0;
}



/*
compression log from make test_compress:

./compressor tobecompressed.txt compressed.c13 -v
settings => verbose = 1, compress = 1, block_size = 16
file_size = 3 bytes, num_blocks = 2
all permutations loaded...
2 blocks to compress...
reading tobecompressed.txt...
done reading tobecompressed.txt...
calculating data for compression...
done calculating data for compression...
creating bit stream that'l be written...
appending num_mapper_entries = 2 (5 bits)
appending max_used_nsb = 7 (5 bits)
appending nsb = 6 (3 bits)
appending average = 4447 (13 bits)
appending max_diff_bits = 2 (4 bits)
appending nsb = 7 (3 bits)
appending average = 5300 (13 bits)
appending max_diff_bits = 2 (4 bits)
mapper size in bits = 53
appending diff = 0 (2 bits)
...was signed? 0
...original: 0
appending diff = 0 (2 bits)
...was signed? 0
...original: 0
bit stream is ready...
Printing blocks:
1st block = 0x7fd5c3c05c70, last_block = 0x7fd5c3c05c70
>>> 2467562449949822976
last block available = 7
writing bit stream to compressed.c13...
byte stream has 8 bytes.
DONE!

*/
