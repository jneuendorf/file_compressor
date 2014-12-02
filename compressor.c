#include "global.h"
#include "permutation.h"
#include "compressor.h"



void init_nsb_data(struct nsb_data *nsb_data, number num_blocks) {
    // TODO: arrays might be way too big! init them with the expected size of num_blocks / (block_size + 1)
    // because right now we create (MAX_NSB * num_blocks) array elements
    nsb_data->indices = calloc(num_blocks, sizeof(number));
    nsb_data->perm_indices = calloc(num_blocks, sizeof(number));
    nsb_data->diffs = calloc(num_blocks, sizeof(signed_number));
}

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


number get_file_size(FILE *file) {
    number file_size;
    fseek(file, 0L, SEEK_END);
    // get file size in bytes
    file_size = ftell(file);
    // seek back to the beginning
    fseek(file, 0L, SEEK_SET);
    return file_size;
}



// from http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
unsigned int number_of_set_bits(int i) {
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

char* bits_to_string(void *p, unsigned int bytes) {
    signed int i, j;
    // int lsb; // least significant bit
    char* res;
    // unsigned long long bits;
    char* byte_p;
    unsigned char byte;

    // cast void pointer to long long pointer and dereference
    // bits = *((unsigned long long *) p);
    byte_p = (char *) p;

    // allocate + clear
    res = calloc(bytes * 8 + 1, sizeof(char));

    if(res != NULL) {
        // iterate through bytes
        for(j = 0; j < bytes; ++j) {
            byte = *byte_p;

            // loop backwards
            for(i = 7; i >= 0; --i) {
                // lsb is 1?
                if((byte & 1) == 1) {
                    res[8*j + i] = '1';
                }
                else {
                    res[8*j + i] = '0';
                }
                // drop lsb
                byte = byte >> 1;
            }
            // point to next char
            byte_p++;
        }
    }

    return res;
}



/*
*
*/
struct bit_stream create_bs(number n, unsigned char used_bits) {
    number *bits;
    unsigned char max_bits;

    // printf("usd bits @create_bs : %d\n", used_bits);

    max_bits = sizeof(number) * 8;

    n = n << (max_bits - used_bits);
    bits = (number *) calloc(1, sizeof(number));
    *bits = n;
    struct bit_stream result = {
        .bits = bits,
        .num_blocks = 1,
        .last_block = bits,
        .avail_bits = max_bits - used_bits,
        .r_block = bits,
        .r_bit_idx = 0
    };
    return result;
}

/*
* used_bits - used bits of *block
*/
void append_num_to_bs(struct bit_stream *bit_stream, number *block, unsigned char used_bits) {
    number max_bits;
    unsigned char available_bits;
    number num_unwanted;

    max_bits = sizeof(number) * 8;

    available_bits = bit_stream->avail_bits;

    // enough space => just append
    if(used_bits <= available_bits) {
        // appending = left shifting + OR (because each block is a number so bits begin right)
        *(bit_stream->last_block) = *(bit_stream->last_block) | (*block << (available_bits - used_bits));
        bit_stream->avail_bits -= used_bits;
    }
    // not enough space => we need to create a new block
    else {
        // printf("new block for %llu\n", *block);
        // printf("avail: %d, num = %llu\n", available_bits, *(bit_stream->last_block));


        // some bits have to go into the previous block and some into the (new) last block

        // number of bits we don't need in the previous block (where a part still fits)
        num_unwanted = used_bits - available_bits;
        // add the first bits that still fit => shift num_unwanted bits out (on the right...they don't fit)
        *(bit_stream->last_block) = *(bit_stream->last_block) | (*block >> num_unwanted);
        // printf("modded prev block = %llu\n", *(bit_stream->last_block));

        // extend array by 1 element
        bit_stream->num_blocks++;
        bit_stream->bits = (number *) realloc(bit_stream->bits, sizeof(number) * bit_stream->num_blocks);

        // go to new block
        // the reallocation might have changed the pointer bit_stream->bits
        // that's why we need to reset bit_stream->last_block
        bit_stream->last_block = bit_stream->bits + (bit_stream->num_blocks - 1);

        // clear memory in new block
        memset(bit_stream->last_block, 0, sizeof(number));

        available_bits = max_bits - num_unwanted;

        // add remaining bits to last block:
        // 1. remove first num_unwanted bits => that leaves us with the bits we want (on the right)
        //    we do that by ANDing with num_unwanted many 1's (2^num_unwanted - 1 == (1 << num_unwanted) - 1)
        // 2. shift remaining bits all the way to the left
        // 3. OR with last block -> but this can be ignored because the memory has been cleared => it's all zeros
        *(bit_stream->last_block) = (((1 << num_unwanted) - 1) & *block) << available_bits;

        // adjust meta data
        bit_stream->avail_bits = available_bits;
    }
}


/*
bit_stream uses number (= 64 bits)
This method tries to make the last block smaller so as few as possible bits are wasted at the end
*/
unsigned char* bs_to_byte_stream(struct bit_stream *bit_stream, number *written_bytes) {
    unsigned char *byte_stream;
    unsigned char *left_block;
    unsigned char *right_block;
    unsigned char swap;
    unsigned char i;
    // unsigned char j;
    number *num_block;
    number max_bits;
    number num_bytes;

    max_bits = sizeof(number) * 8;

    // number of bytes to copy = bytes_in_number * (num_blocks-1) + ceil(used_bits of last block / 8)
    // ceil(x/y) = 1 + ((x - 1) / y); // if x != 0
    // num_bytes = (bit_stream->num_blocks - 1) * sizeof(number) + (max_bits - bit_stream->avail_bits + 1) / 8 + 1;
    num_bytes = (bit_stream->num_blocks - 1) * sizeof(number) + CEIL_X_DIV_Y(max_bits - bit_stream->avail_bits, 8);

    // allocate memory
    byte_stream = (unsigned char *) calloc(num_bytes, sizeof(unsigned char));


    // copy memory from second pointer to the first
    // printf("copying %llu bytes...\n", num_bytes);
    // printf("first num = %llu -> %s\n", bit_stream->bits[0], bits_to_string(bit_stream->bits, 8));
    // number n = 10804907740292013867;
    // printf("-> %s\n", bits_to_string(&n, 8));


    // number blocks are saved with LSB on the left (= lowest memory address)!
    if(!is_big_endian()) {
        // go through all number blocks
        num_block = bit_stream->bits;
        do {
            // point to outermost blocks
            left_block = (unsigned char *) num_block;
            right_block = left_block + sizeof(number) - 1;


            // for each number block
            // go through half of the byte blocks of the number block and swap with other half
            for(i = 0; i < (sizeof(number) >> 1); ++i) {
                swap = *left_block;
                // printf()
                *left_block = *right_block;
                *right_block = swap;

                // move pointers inward
                left_block++;
                right_block--;
            }
        } while(num_block++ != bit_stream->last_block);
    }

    memcpy(byte_stream, bit_stream->bits, num_bytes);

    *written_bytes = num_bytes;

    return byte_stream;
}

// deallocate with realloc(ptr, 0)

number get_bs_size(struct bit_stream *bit_stream) {
    return bit_stream->num_blocks * sizeof(number) * 8 - bit_stream->avail_bits;
}

number read_bs(struct bit_stream *bit_stream, unsigned char num_bits) {
    number result;

    result = 0;

    // read as long as we want to read more and we're not passed the end
    while(num_bits > 0 && bit_stream->r_block != bit_stream->last_block) {



        // go to next block
        (bit_stream->r_block)++;
    }

    return result;
}

// http://graphics.stanford.edu/~seander/bithacks.html#SwappingBitsXOR
// Swapping individual bits with XOR

// unsigned int i, j; // positions of bit sequences to swap
// unsigned int n;    // number of consecutive bits in each sequence
// unsigned int b;    // bits to swap reside in b
// unsigned int r;    // bit-swapped result goes here
//
// unsigned int x = ((b >> i) ^ (b >> j)) & ((1U << n) - 1); // XOR temporary
// r = b ^ ((x << i) | (x << j));
// As an example of swapping ranges of bits suppose we have have b = 00101111 (expressed in binary) and we want to swap the n = 3 consecutive bits starting at i = 1 (the second bit from the right) with the 3 consecutive bits starting at j = 5; the result would be r = 11100011 (binary).
// This method of swapping is similar to the general purpose XOR swap trick, but intended for operating on individual bits.  The variable x stores the result of XORing the pairs of bit values we want to swap, and then the bits are set to the result of themselves XORed with x.  Of course, the result is undefined if the sequences overlap.
