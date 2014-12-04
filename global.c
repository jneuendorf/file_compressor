#include "global.h"


// from: http://www.geeksforgeeks.org/space-and-time-efficient-binomial-coefficient/
// Returns value of Binomial Coefficient C(n, k)
number binom(unsigned char n, unsigned char k) {
    number res;
    unsigned char i;

    res = 1;

    // Since C(n, k) = C(n, n-k)
    if(k > n - k) {
        k = n - k;
    }

    // Calculate value of [n * (n-1) *---* (n-k+1)] / [k * (k-1) *----* 1]
    for(i = 0; i < k; ++i) {
        res *= (n - i);
        res /= (i + 1);
    }

    return res;
}


void parse_cmd_line_arguments(int argc, char const *argv[], struct settings *settings) {
    unsigned char block_size;

    // default values
    settings->compress = true;
    settings->verbose = false;
    settings->block_size = 16;
    settings->memory_block_size = 256;

    // get command line arguments
    if(argc > 3) {
        for(unsigned char i = 3; i < argc; ++i) {
            if(strcmp("-v", argv[i]) == 0 || strcmp("-v", argv[i]) == 0) {
                settings->verbose = true;
            }
            else if(strcmp("--decompress", argv[i]) == 0) {
                settings->compress = false;
            }
            else if(strcmp("-b", argv[i]) == 0) {
                block_size = strtol(argv[++i], NULL, 10);
                // set only if valid
                if(block_size >= 8 && block_size <= 64 && block_size % 8 == 0) {
                    settings->block_size = block_size;
                }
            }
            else if(strcmp("-m", argv[i]) == 0) {
                block_size = strtol(argv[++i], NULL, 10);
                // set only if valid
                if(block_size != 0) {
                    settings->memory_block_size = block_size;
                }
            }
        }
    }
    // set greatest possible NSB (block_size + 1 because from 0 to block_size)
    settings->max_nsb = settings->block_size + 1;
    // set greatest possible permutation index
    settings->max_perm_idx = binom(settings->block_size, settings->block_size / 2);
    // set greatest possible average permutation index (half of max. permutation index)
    settings->max_avg_idx = settings->max_perm_idx / 2;
    // convert memory_block_size from mega bytes to bytes
    settings->memory_block_size *= 1048576;

    D(printf("settings-> verbose = %d, compress = %d, block_size = %u\n", settings->verbose, settings->compress, settings->block_size);)
}
