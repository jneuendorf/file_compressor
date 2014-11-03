#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "global.h"
#include "compressor.h"



int main (int argc, char const *argv[]) {
    FILE            *file;
    char            buffer[2];
    // unsigned int    zero_count;
    // unsigned int    one_count;
    unsigned char   i; // type must match type of struct sequence.pattern_id
    // unsigned int    set_bits;
    // unsigned int    sequence_length;
    struct sequence seq;
    bool            exit_loop;
    // bool            has_next_permutation = true;


    // initialize patterns
    define_patterns();

    // printf("%d\n", patterns[1].matches(NULL));


    // number of bits to permute (default -> 2 bytes)
    seq.length = 16;
    // sequence_length = 16; // steps: 16 -> 3217, 32 -> 150270097, 64 -> 4.58156e17; max. 64
    // TODO: maybe:
    // TODO: make this number bigger by having an array of longs and simulate a shift in the whole array!

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


    if(argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    printf("int size: %lu bytes\n", sizeof(char));

    // printf("%d\n", argc);

    // try to open the file
    file = fopen(argv[1], "rb");
    if(file == NULL)  {
        printf("Error: Can't open '%s' for read!\n", argv[1]);
        return 1;
    }

    // zero_count = 0;
    // one_count = 0;

    // Hier wird die LeseDatei solange durchlaufen bis das Dateiende erreicht (-> feof = end of file)
    while(!feof(file))    {
        printf("beginning of read loop\n");

        // read next n bytes (+1 because '\0' is also appended)
        fgets(buffer, seq.length / 8 + 1, file);

        // printf("buffer address: %d\n", buffer);

        // set_bits = number_of_set_bits(buffer[0]);
        // one_count += set_bits;
        // zero_count += (8 - set_bits);
        // printf("%s=%d - %d,\n", buffer, buffer[0], number_of_set_bits(buffer[0]));

        printf("buffer: %s (%lu)\n", buffer, strlen(buffer));

        seq.chars = bits_to_string(buffer, seq.length / 8);


        exit_loop = false;

        do {
            // printf("%d\n", seq.chars);
            printf("sequence: %s (%lu)\n", seq.chars, strlen(seq.chars));
            // check if current sequence matches a pattern
            for(i = 0; i < num_patterns; ++i) {
                if(patterns[i].matches(&seq)) {
                    seq.pattern_id = i;
                    exit_loop = true;
                    break;
                }
            }
            printf("end of do-while loop\n");
            has_next_permutation = permute(seq.chars, seq.length);
        // } while(!exit_loop && permute(seq.chars, seq.length));
            printf("has_next_permutation: %d\n", has_next_permutation);
        } while(!exit_loop && has_next_permutation);


        // sequence = bits_to_string(buffer[0], sequence_length);
        // printf("...%s\n", sequence);
    }

    printf("yeah!!!!!\n");


    fclose(file);




    return 0;
}
