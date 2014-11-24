#include <stdint.h>
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


//////////////////////////////////////////////////////////////////


int main (int argc, char const *argv[]) {
    FILE                *file;
    char                *buffer;
    unsigned char       i; // type must match type of struct sequence.pattern_id
    struct sequence     seq;
    number              block_size;
    number              nsb; // number of set bits
    bool                exit_loop;
    number              *bits;
    // define indices of pattern (=number) of lexicographical order of all permutations
    // FOR EACH number_of_set_bits! (n^2)
    // number           indices[MAX_NSB][BLOCK_SIZE];
    number              nsb_permutations[MAX_NSB][BLOCK_SIZE];
    number              permutation;
    number              perm_idx;
    struct mapper_entry mapper[MAX_NSB];
    struct nsb_data     nsb_datas[MAX_NSB];


    // max. 64
    block_size = 16;

    buffer = calloc(block_size / 8 + 1, sizeof(char));


    if(argc < 3) {
        printf("Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }


    // try to open the file
    file = fopen(argv[1], "rb");
    if(file == NULL)  {
        printf("Error: Can't open '%s' for read!\n", argv[1]);
        return 1;
    }

    // TODO: get file size (for knowing remaining bits and progress)

    // 0. fill memory with permutations for each nsb (TODO: this could be done dynamically so unneeded nsb's are not computed)
    //    also initialize all the pointers of struct nsb_data
    for(nsb = 0; nsb < MAX_NSB; ++nsb) {
        // permutations
        permutation = (1 << nsb) - 1; // 2^nsb - 1 = nsb many 1s (= the first permutation for that many set bits)
        perm_idx = 0;
        do {
            nsb_permutations[nsb][perm_idx++] = permutation;
            // ++perm_idx; // inlined above
        } while(next_permutation_bitwise(&permutation, block_size));
        // nsb_data pointers
        init_nsb_data(&nsb_datas[nsb], block_size);
    }


    // 1. read entire file and gather data needed for compression
    // read file til its end (-> feof = end of file)
    while(!feof(file))    {
        printf("beginning of read loop\n");

        // read next n bytes (+1 because '\0' is also appended)
        fgets(buffer, block_size / 8 + 1, file);

        // set_bits = number_of_set_bits(buffer[0]);

        printf("buffer: %s (%lu)\n", buffer, strlen(buffer));

        // seq.chars = bits_to_string(buffer, seq.length / 8);

        bits = (number *) buffer;

        // printf("%d\n", *bits);



        // go to next block
        // bits++;
    }

    printf("yeah!!!!!\n");


    fclose(file);

    return 0;
}
