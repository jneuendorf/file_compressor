#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "global.h"
#include "permutation.h"
#include "compressor.h"



int main (int argc, char const *argv[]) {
    FILE            *file;
    char            *buffer;
    unsigned char   i; // type must match type of struct sequence.pattern_id
    struct sequence seq;
    number          sequence_length;
    bool            exit_loop;
    number          *bits;


    // initialize patterns
    define_patterns();


    // number of bits to permute (default -> 2 bytes)
    // steps in dependence of number of bits:
    // 8 ->                       17.5
    // 16 ->                    3217.5
    // 24 ->                 676 039
    // 32 ->             150 270 097
    // 40 ->          34 461 632 205
    // 48 ->       8 061 900 920 775.16
    // 56 ->   1 912 172 650 190 140.2
    // 64 -> 458 156 035 235 661 120
    // max. 64
    // seq.length = 16;
    sequence_length = 16;
    // TODO: maybe:
    // TODO: make this number bigger by having an array of longs and simulate a shift in the whole array!

    buffer = calloc(sequence_length / 8 + 1, sizeof(char));


    // char test[3] = "abc";
    //
    // permute(test, 3);
    //
    // printf("permute: %s\n", test);
    //
    // unpermute(test, 3);
    //
    // printf("unpermute: %s\n", test);


    // testing sorted pattern
    // struct sequence test_sequence = {.chars = "1111111111100000", .length = 16};
    // printf("%s\n", "here we go");
    // int temp = permute("0000111", 7);
    // printf("%d\n", temp);

    // temp = permute("1111111111100000", 16);
    // printf("%d\n", temp);
    // return 1;
    // printf(">> sorted? %d - %d\n", pattern_is_sorted(&test_sequence), pattern_is_sorted_inverse(&test_sequence));


    // char test2[] = "mynameisjim";
    // printf("bits: %s\n", bits_to_string(test2, 11));


    if(argc < 3) {
        printf("Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }

    // printf("int size: %lu bytes\n", sizeof(char));

    // printf("%d\n", argc);

    // try to open the file
    file = fopen(argv[1], "rb");
    if(file == NULL)  {
        printf("Error: Can't open '%s' for read!\n", argv[1]);
        return 1;
    }

    // TODO: get file size (for knowing remaining bits and progress)


    // Hier wird die LeseDatei solange durchlaufen bis das Dateiende erreicht (-> feof = end of file)
    while(!feof(file))    {
        printf("beginning of read loop\n");

        // read next n bytes (+1 because '\0' is also appended)
        fgets(buffer, sequence_length / 8 + 1, file);

        // set_bits = number_of_set_bits(buffer[0]);

        printf("buffer: %s (%lu)\n", buffer, strlen(buffer));

        // seq.chars = bits_to_string(buffer, seq.length / 8);

        bits = (number *) buffer;

        // printf("%d\n", *bits);

        while(next_permutation_bitwise(bits, strlen(buffer) * 8)) {
            printf("%d\n", *bits);
        }

        printf("%d\n", pattern_is_sorted_inverse(*bits));

        return 1;

        exit_loop = false;

        do {
            // printf(">> %d\n", *bits);

            // check if current sequence matches a pattern
            // checking sorted patterns does not make sense because:
            // sorted means         -> 1. permutation (probably never gonna be reached)
            // inverse sorted means -> last permutation (only the case after the loop)

            // uneven bits -> there is a pattern like (10)*1 at index 20/34 = 0.5882, 76/125 = 0.608
            // but: bits in our cases (bytes!) are always 8n -> even
            // even bits -> pattern like (10)+ at 49/69 = 0.71 and pattern like (01)+ at 20/69 = 0.289

            if(pattern_is_sorted(*bits)) {
                printf("sorted!\n");
                exit_loop = true;
            }
            else if(pattern_is_sorted_inverse(*bits)) {
                printf("invert sorted!\n");
                exit_loop = true;
            }

            // for(i = 0; i < num_patterns; ++i) {
            //     // if(patterns[i].matches(&seq)) {
            //     //     seq.pattern_id = i;
            //     //     printf("%llu\n", seq.n);
            //     //     exit_loop = true;
            //     //     break;
            //     // }
            // }
            // printf("end of do-while loop\n");
            // printf("has_next_permutation: %d\n", has_next_permutation);
        // } while(!exit_loop && has_next_permutation);
        } while(!exit_loop && next_permutation_bitwise(bits, strlen(buffer) * 8));


        // go to next block
        // bits++;
    }

    printf("yeah!!!!!\n");


    fclose(file);

    return 0;
}
